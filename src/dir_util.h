#include <sys/stat.h>
#include<errno.h>
#include<string.h>
#include<stdio.h>
#include<stdlib.h>

/**
 * Adapted from: https://gist.github.com/JonathonReinhart/8c0d9019c38af2dcadb102c4e202950
*/
int mkdir_p (const char *path);
int execute_mkdir (const char *path, mode_t mode);

int execute_mkdir(const char *path, mode_t mode) {
    struct stat st;

    int ret;

    errno = 0;

    ret = mkdir(path, mode);
    if (ret == 0) {
        return ret;
    }

    if (ret == EEXIST) {
        return -1;
    }

    ret = stat(path, &st);
    if (ret != 0) {
        return -1;
    }

    if (!S_ISDIR(st.st_mode)) {
        errno = ENOTDIR;
        return -1;
    }

    return 0;
}

int mkdir_p(const char *path) {
    char *_path;
    char *p;
    int ret = -1;
    mode_t mode = 0777;

    _path = strdup(path);
    if (_path == NULL) {
        fprintf(stderr, "failed duplicating string\n");
        return ret;
    }

    for (p = _path + 1; *p; p++) {
        if (*p == '/') {
            *p = '\0';

            if (mkdir(_path, mode) != 0) {
                goto out;
            }

            *p = '/';

        }
    }

    if (mkdir(_path, mode) != 0) {
        goto out;
    }

    ret = 0;

out:
   free(_path);
   return ret;
}