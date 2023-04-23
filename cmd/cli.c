#define _GNU_SOURCE
#include<stdio.h>
#include<fcntl.h>
#include<unistd.h>
#include<stdlib.h>
#include<string.h>
#include<errno.h>
#include "../src/archive_wrapper.h"
#include<uuid/uuid.h>

int main(int argc, const char **argv)
{

    char **input_files = NULL;

    int el = 0;

    int cap = 10;

    if (argc < 2) {
        input_files = malloc(cap * sizeof(char*) );

        char *buf = NULL;
        size_t len = 0;
        ssize_t nread;

        while((nread = getline(&buf, &len, stdin)) != -1) {
            input_files[el] = malloc(len * sizeof(char*));
            strcpy(input_files[el], buf);
            input_files[el][strlen(input_files[el]) - 1] = '\0';
            el++;
            if (el >= cap) {
                cap *= 2;
                input_files = (char **)realloc(input_files, cap * sizeof(char*));
             }
        }

    }
    
    uuid_t outfile_t;

    char *ext = ".tar.gz";
    
    if (input_files != NULL) {

        uuid_generate(outfile_t);

        size_t out_len = 37 + strlen(ext);

        char *outfile = (char *)malloc(out_len * sizeof(char*));

        uuid_unparse_lower(outfile_t, outfile);

        strcat(outfile, ext);

        printf("output tar.gz to: %s\n", outfile);
        
        int ret = gzip(el, (const char **)input_files, (const char *)outfile);
        if (ret != 0) {
            fprintf(stderr, "failed creating tar.gz files\n");
            return -1;
        }
    }

    return 0;
}
