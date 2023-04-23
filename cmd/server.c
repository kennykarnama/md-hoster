#include<microhttpd.h>
#include<string.h>
#include<stdio.h>
#include<stdlib.h>
#include<errno.h>
#include"../src/archive_wrapper.h"

#define PORT 8080
#define GET 0
#define POST 1
#define POST_BUF_SIZE 10240

const char *internal_error_page = "<html><body><h1>An Internal Server Error happens!</h1></body></html>";
const char *file_already_exist = "<html><body><h1>File already exist!</h1></body></html>";
const char *success_msg = "<html><body><h1>Upload success</h1></body></html>";
const char *not_found_msg = "<html><body><h1> Page Not found<h1></body></html>";

struct connection_info {
    int connectionType;
    struct MHD_PostProcessor *postProcessor;
    FILE *fp;
    const char *resp;
    int code;
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

        con_info->fp = fopen(filename, "wb");
        if (con_info->fp == NULL) {
            fprintf(stderr, "failed opening: %s err: %s\n", filename, strerror(errno));
            return MHD_NO;
        }
    }

    if (size > 0) {
        int nwrite = fwrite(data, size, sizeof(char), con_info->fp);
        if (!nwrite) {
            fprintf(stderr, "failed to write file\n");
            return MHD_NO;
        }
        fflush(con_info->fp);
        fclose(con_info->fp);
        con_info->fp = NULL;
        
        // extract
        printf("extract file: %s\n", filename);
        int ret = extract_archive(filename);
        if (ret < ARCHIVE_OK) {
            fprintf(stderr, "extract failed. remove file: %s\n", filename);
            remove(filename);
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
        }
        if (con_info->postProcessor != NULL) {
            MHD_destroy_post_processor(con_info->postProcessor);
        }
    }

    free(con_info);
    *conn_cls = NULL;
}

static enum MHD_Result render_response(struct MHD_Connection *conn, const char *page, int status_code) {
    int ret;
    struct MHD_Response *resp;

    resp = MHD_create_response_from_buffer(strlen(page), (void *)page, MHD_RESPMEM_PERSISTENT);
    if (resp == NULL) {
        return MHD_NO;
    }
    MHD_add_response_header(resp, MHD_HTTP_HEADER_CONTENT_TYPE, "text/html");
    ret = MHD_queue_response(conn, status_code, resp);
    MHD_destroy_response(resp);
    return ret;
}

enum MHD_Result route(void *cls, struct MHD_Connection *connection, const char *url, const char *method, const char *version,
const char *upload_data, size_t *upload_data_size, void **con_cls) {
    
    printf("received request with URL: %s\n", url);

    struct MHD_Response *resp = NULL;

    int ret;

    int http_status;

    if (*con_cls == NULL) {
        struct connection_info *con_info = malloc(sizeof (struct connection_info));

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
            return MHD_NO;
        }

        *con_cls = (void *)con_info;

        return MHD_YES;
    }


    
    if (strcmp(method, MHD_HTTP_METHOD_POST) == 0 && strcmp(url, "/upload") == 0) {
        struct connection_info *con_info = *con_cls;
        if (*upload_data_size > 0) {
            MHD_post_process(con_info->postProcessor, upload_data, *upload_data_size);
            *upload_data_size = 0;
            return MHD_YES;
        }else {
            if (con_info->fp != NULL) {
                fclose(con_info->fp);
                con_info->fp = NULL;
            }
            ret = render_response(connection, con_info->resp, con_info->code);
            return ret;
        }
        
    }

    return MHD_NO;
}


int main(int argc, char **argv) {
    struct MHD_Daemon *daemon;

    daemon = MHD_start_daemon(MHD_USE_INTERNAL_POLLING_THREAD, PORT, NULL, NULL, &route, NULL, 
                              MHD_OPTION_NOTIFY_COMPLETED, request_completed, NULL, MHD_OPTION_END);

    if (daemon == NULL) {
        fprintf(stderr, "unable to initialize mhd daemon\n");
        return -1;
    }

    printf("starting server on port: %d\n", PORT);

    getchar();

    MHD_stop_daemon(daemon);

    return 0;
}