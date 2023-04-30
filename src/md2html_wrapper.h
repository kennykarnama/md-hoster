#ifndef MD2HTML_WRAPPER_H
#define MD2HTML_WRAPPER_H

#include<md4c-html.h>
#include<stdio.h>
#include<fcntl.h>
#include<unistd.h>
#include<string.h>
#include<errno.h>
#include"simple_membuf.h"

char* 
render_md_html(const char *input);

void
process_output(const MD_CHAR* text, MD_SIZE size, void* userdata);

void
process_output(const MD_CHAR* text, MD_SIZE size, void* userdata)
{
    membuf_append((struct membuffer*) userdata, text, size);
}

char* 
render_md_html(const char *input) 
{

    printf("processing input: %s\n", input);

    // read all file contents to buffer
    int fd;
    fd = open(input, O_RDONLY);
    if (fd == -1) {
        fprintf(stderr, "render_md_html.open file: %s err: %s\n", input, strerror(errno));
        return NULL;
    }

    off_t end_off = lseek(fd, 0, SEEK_END);
    off_t begin_off = lseek(fd, 0, SEEK_SET);
    size_t buf_len = begin_off + end_off + 1;
    char buf[buf_len];
    int n_read = read(fd, buf, buf_len);

    if (n_read == -1) {
        fprintf(stderr, "render_md_html.read file: %s err: %s\n", input, strerror(errno));
        return NULL;
    }
    buf[buf_len-1] = '\0';

    struct membuffer buf_out = {0};

    membuf_init(&buf_out, 1024);

    int ret = md_html(buf, strlen(buf), process_output, (void *)&buf_out, MD_DIALECT_GITHUB, MD_HTML_FLAG_VERBATIM_ENTITIES);
    if (ret == -1) {
        fprintf(stderr, "md_html parsed failed\n");
        return NULL;
    }
    
    struct membuffer result = {0};

    membuf_init(&result, 1024);

    char *tobe_rendered[] = {
        "<!DOCTYPE html>\n",
        "<html>\n",
        "<head>\n",
        "<title></title>\n",
        "</head>\n",
        "<body>\n",
        "<content>\n", // replace this with buf_out content
        "</body>\n",
        "</html>\n"
    };

    size_t rendered_el = sizeof(tobe_rendered) / sizeof(char *);

    for (int iter = 0; iter < rendered_el; iter++) {
        if (strcmp(tobe_rendered[iter], "<content>\n") == 0) {
            membuf_append(&result, buf_out.data, buf_out.size);
        }else {
            membuf_append(&result, tobe_rendered[iter], strlen(tobe_rendered[iter]));
        }
    }
    
    char *dst = malloc(strlen(result.data) + 1);
    strcpy(dst, result.data);
    dst[strlen(dst)-1] = '\0';

    membuf_fini(&result);
    membuf_fini(&buf_out);

    return dst;
}

#endif