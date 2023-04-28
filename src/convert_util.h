#ifndef CONVERT_UTIL_H
#define CONVERT_UTIL_H

#include<inttypes.h>
#include<errno.h>

int
str_to_uint16(const char *str, uint16_t *res);


int
str_to_uint16(const char *str, uint16_t *res)
{
    char *end;
    errno = 0;
    intmax_t val = strtoimax(str, &end, 10);
    if (errno == ERANGE || val < 0 || val > UINT16_MAX || end == str || *end != '\0') {
        return 0;
    }
    *res = (uint16_t) val;
    return 1;
}

#endif