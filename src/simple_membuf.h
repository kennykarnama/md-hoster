#ifndef SIMPLE_MEMBUF_H
#define SIMPLE_MEMBUF_H

#include<stdio.h>
#include<stdlib.h>
#include<string.h>

struct membuffer {
    char* data;
    size_t asize;
    size_t size;
};

void
membuf_init(struct membuffer* buf, size_t new_asize);

void
membuf_fini(struct membuffer* buf);

void
membuf_grow(struct membuffer* buf, size_t new_asize);

void
membuf_append(struct membuffer* buf, const char* data, size_t size);

void
membuf_init(struct membuffer* buf, size_t new_asize)
{
    buf->size = 0;
    buf->asize = new_asize;
    buf->data = (char *)calloc(buf->asize, sizeof(char));
    if(buf->data == NULL) {
        fprintf(stderr, "membuf_init: malloc() failed.\n");
        exit(1);
    }
}

void
membuf_fini(struct membuffer* buf)
{
    if(buf->data)
        free(buf->data);
}

void
membuf_grow(struct membuffer* buf, size_t new_asize)
{
    buf->data = (char *)realloc(buf->data, new_asize);
    if(buf->data == NULL) {
        fprintf(stderr, "membuf_grow: realloc() failed.\n");
        exit(1);
    }
    buf->asize = new_asize;
}

void
membuf_append(struct membuffer* buf, const char* data, size_t size)
{
    if(buf->asize < buf->size + size)
        membuf_grow(buf, buf->size + buf->size / 2 + size);
    memcpy(buf->data + buf->size, data, size);
    buf->size += size;
}

#endif