#ifndef FILE_UTIL_H
#define FILE_UTIL_H

#include<stdio.h>
#include<unistd.h>
#include<fcntl.h>
#include<string.h>
#include<errno.h>
#include<stdlib.h>
#include<sys/stat.h>

struct binary_data {
    size_t len;
    char *content;
};

char* 
readfile(const char *in);

struct binary_data
readfile_binary(const char *in);

char* 
readfile(const char *in) 
{
    int fd;
    fd = open(in, O_RDONLY);
    if (fd == -1) {
        fprintf(stderr, "render_md_html.open file: %s err: %s\n", in, strerror(errno));
        return NULL;
    }

    off_t end_off = lseek(fd, 0, SEEK_END);
    off_t begin_off = lseek(fd, 0, SEEK_SET);
    size_t buf_len = begin_off + end_off + 1;
    char buf[buf_len];
    int n_read = read(fd, buf, buf_len);

    if (n_read == -1) {
        fprintf(stderr, "render_md_html.read file: %s err: %s\n", in, strerror(errno));
        return NULL;
    }
    buf[buf_len-1] = '\0';

    char *s_ptr = buf;

    return s_ptr; 
}

struct binary_data
readfile_binary(const char *in)
{
    struct stat st;

    struct binary_data bd;

    int fd = open(in, O_RDONLY);

    if (fd == -1) {
        fprintf(stderr, "failed opening file: %s err: %s\n", in, strerror(errno));
        return bd;
    }

    fstat(fd, &st);

    char *buf = (char *)malloc((st.st_size + 10) * sizeof(char) );

    int nread = read(fd, buf, st.st_size +1);

    if (nread == -1) {
        fprintf(stderr, "failed reading file: %s err: %s\n", in, strerror(errno));
        return bd;
    }

    printf("file size in bytes: %lld\n", st.st_size);

    close(fd);

    bd.len = st.st_size;
    bd.content = buf;

    return bd;
}

#endif