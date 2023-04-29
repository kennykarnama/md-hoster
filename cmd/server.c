#include<microhttpd.h>
#include<string.h>
#include<stdio.h>
#include<stdlib.h>
#include<errno.h>
#include<unistd.h>
#include<signal.h>
#include"../src/archive_wrapper.h"
#include"../src/dir_util.h"
#include"../src/regex_wrapper.h"
#include"../src/md2html_wrapper.h"
#include"../src/file_util.h"
#include"../src/convert_util.h"

#define PORT "8080"
#define GET 0
#define POST 1
#define POST_BUF_SIZE 512
#define MD_DIR "md/out"

struct MHD_Daemon *server = NULL;

const char *internal_error_page = "<html><body><h1>An Internal Server Error happens!</h1></body></html>";
const char *file_already_exist = "<html><body><h1>File already exist!</h1></body></html>";
const char *success_msg = "<html><body><h1>Upload success</h1></body></html>";
const char *not_found_msg = "<html><body><h1> Page Not found<h1></body></html>";
const char *md_view_page = "<html><body><h1>MD view page</h1></body></html>";
char *md_render_route_pattern = NULL;


static void
signal_handler(int sig)
{
    if (sig == SIGINT) {
        
        printf("SIGINT received\n");

        if (server != NULL) {
            printf("stopping server daemon...\n");
            MHD_stop_daemon(server);
        }

        free(md_render_route_pattern);

        exit(EXIT_SUCCESS);
    }
}

struct connection_info {
    int connectionType;
    struct MHD_PostProcessor *postProcessor;
    FILE *fp;
    const char *resp;
    int code;
    int renderPage;
    char *filename;
};

static enum MHD_Result
iterate_post(void *coninfo_cls, enum MHD_ValueKind kind, const char *key, const char *filename, const char *content_type,
             const char *transfer_encoding, const char *data, uint64_t offset, size_t size
) {
    struct connection_info *con_info = coninfo_cls;
    FILE *fp;

    con_info->code = MHD_HTTP_INTERNAL_SERVER_ERROR;
    con_info->resp = internal_error_page;

    if (strcmp(key, "file") != 0) {
        return MHD_NO;
    }

    if (con_info->fp == NULL) {
        fp = fopen(filename, "rb");
        if (fp != NULL) {
            fclose(fp);
            con_info->resp = file_already_exist;
            return MHD_NO;
        }

        con_info->fp = fopen(filename, "ab");
        if (con_info->fp == NULL) {
            fprintf(stderr, "failed opening: %s err: %s\n", filename, strerror(errno));
            return MHD_NO;
        }
        con_info->filename = calloc(strlen(filename), sizeof(char));
        strcpy(con_info->filename, filename);
    }

    fprintf(stderr, "size: %d\n", size);
    
    if (size > 0) {
        
        int nwrite = fwrite(data, size, sizeof(char), con_info->fp);
        if (!nwrite) {
            fprintf(stderr, "failed to write file\n");
            return MHD_NO;
        }
    }

    con_info->resp = success_msg;
    con_info->code = MHD_HTTP_CREATED;

    return MHD_YES;
}

static void
request_completed(void *cls, struct MHD_Connection *connection, void **conn_cls, enum MHD_RequestTerminationCode toe)
{
    struct connection_info *con_info = *conn_cls;
    if (con_info == NULL) {
        return;
    }

    if (con_info->connectionType == POST) {
        if (con_info->fp != NULL) {
            fclose(con_info->fp);
            con_info->fp = NULL;
        }
        if (con_info->postProcessor != NULL) {
            MHD_destroy_post_processor(con_info->postProcessor);
        }
        if (con_info->filename != NULL) {
            free(con_info->filename);
            con_info->filename = NULL;
        }
    }

    free(con_info);
    *conn_cls = NULL;
}

static enum MHD_Result render_response(struct MHD_Connection *conn, const char *page, int status_code, 
const char *content_type, size_t content_len) {
    int ret;
    struct MHD_Response *resp;

    resp = MHD_create_response_from_buffer(content_len, (void *)page, MHD_RESPMEM_MUST_FREE);
    if (resp == NULL) {
        return MHD_NO;
    }
    MHD_add_response_header(resp, MHD_HTTP_HEADER_CONTENT_TYPE, content_type);
    ret = MHD_queue_response(conn, status_code, resp);
    MHD_destroy_response(resp);
    return ret;
}

static enum MHD_Result route(void *cls, struct MHD_Connection *connection, const char *url, const char *method, const char *version,
const char *upload_data, size_t *upload_data_size, void **con_cls) {
    
    printf("received request with URL: %s\n", url);

    struct MHD_Response *resp = NULL;

    int ret;

    int http_status;

    if (*con_cls == NULL) {
        struct connection_info *con_info = malloc(sizeof (struct connection_info));
        con_info->fp = NULL;
        con_info->postProcessor = NULL;
        con_info->filename = NULL;

        if (con_info == NULL) {
            fprintf(stderr, "failed allocating");
            return MHD_NO;
        }

        if (strcmp(MHD_HTTP_METHOD_POST, method) == 0) {
            if (strcmp(url, "/upload") == 0) {
                con_info->connectionType = POST;
                con_info->postProcessor = MHD_create_post_processor(connection, POST_BUF_SIZE, iterate_post, (void *)con_info);
            
                if (con_info->postProcessor == NULL) {
                 free(con_info);
                 return MHD_NO;
                }

                con_info->code = MHD_HTTP_CREATED;
                con_info->resp = success_msg;
            }
        }else {
            if (match(url, md_render_route_pattern) != 1) {
                free(con_info);
                return MHD_NO;
            }

            con_info->connectionType = GET;
            con_info->renderPage = 1;
        
        }

        *con_cls = (void *)con_info;

        return MHD_YES;
    }


    
    if (strcmp(method, MHD_HTTP_METHOD_POST) == 0 && strcmp(url, "/upload") == 0) {
        struct connection_info *con_info = *con_cls;
        if (*upload_data_size > 0) {
            enum MHD_Result res = MHD_post_process(con_info->postProcessor, upload_data, *upload_data_size);
            printf("post_process.... result: %d\n", res);
            *upload_data_size = 0;
            return MHD_YES;
        }else {
            if (con_info->fp != NULL) {
                fflush(con_info->fp);
                fclose(con_info->fp);
                con_info->fp = NULL;
                // extract
                printf("extract file: %s\n", con_info->filename);
                int ret = extract_archive(con_info->filename, MD_DIR);
                if (ret < ARCHIVE_OK) {
                   fprintf(stderr, "extract failed. remove file: %s\n", con_info->filename);
                   remove(con_info->filename);
                   return MHD_NO;
                }
            }
            ret = render_response(connection, con_info->resp, con_info->code, "text/html", strlen(con_info->resp));
            return ret;
        }
    } else if (strcmp(method, MHD_HTTP_METHOD_GET) == 0) {
        struct connection_info *con_info = *con_cls;
        if (con_info->renderPage) {
            char *result = NULL;
            char *ct = "text/html";
            size_t ctl = 0;

            if (strstr(url, ".md") != NULL) {
                result = render_md_html(url + 1);
                ctl = strlen(result);
            }else if (strstr(url, ".jpg") != NULL || strstr(url, ".jpeg") != NULL) {
                ct = "image/jpeg";
                struct binary_data bd = readfile_binary(url + 1);
                ctl = bd.len;
                result = bd.content;
            }
            
            if (result == NULL) {
                ret = render_response(connection, internal_error_page, MHD_HTTP_INTERNAL_SERVER_ERROR, 
                "text/html", strlen(internal_error_page));
                return ret;
            }
            ret = render_response(connection, result, MHD_HTTP_OK, ct, ctl);
            return ret;
        }else {
              ret = render_response(connection, internal_error_page, MHD_HTTP_NOT_FOUND, 
                "text/html", strlen(not_found_msg));
                return ret;
        }
    }

    return render_response(connection, internal_error_page, MHD_HTTP_NOT_FOUND, 
                "text/html", strlen(internal_error_page));
                return ret;
}


int main(int argc, char **argv) {

    printf("bootstrap server assets\n");

    int ret = mkdir_p(MD_DIR);
    if (ret != 0) {
        fprintf(stderr, "bootstrap.dir err: %s\n", strerror(errno));
    }

    size_t route_pattern_len = strlen(MD_DIR) + strlen("/[a-zA-Z0-9_.-]*") + 1;

    md_render_route_pattern = (char *)malloc(route_pattern_len * sizeof(char*));

    if (md_render_route_pattern == NULL) {
        fprintf(stderr, "failed allocating memory for render_route_pattern\n");
        return -1;
    }

    sprintf(md_render_route_pattern, "%s%s", MD_DIR, "/[a-zA-Z0-9_.-]*");

    printf("route pattern: %s\n", md_render_route_pattern);

    char *env_port = getenv("PORT");
    if (env_port == NULL) {
        env_port = PORT;
    }

    uint16_t port = 0;
    if (!str_to_uint16((const char*)env_port, &port)){
        fprintf(stderr, "failed to parse port\n");
        return -1;
    }


    server = MHD_start_daemon(MHD_USE_DEBUG | MHD_USE_INTERNAL_POLLING_THREAD, port, NULL, NULL, &route, NULL, 
                              MHD_OPTION_NOTIFY_COMPLETED, request_completed, NULL, MHD_OPTION_END);

    if (server == NULL) {
        fprintf(stderr, "unable to initialize mhd daemon\n");
        return -1;
    }

    printf("starting server on port: %d\n", port);

    if (signal(SIGINT, signal_handler)) {
        return 0;
    }

    for (;;) {
        pause();
    }

    return 0;
}