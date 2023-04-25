#include<regex.h>
#include<stdio.h>
#include<stdlib.h>

int 
match(const char *target, const char *pattern);
char*
get_regerror(int errcode, regex_t *rg);

int 
match(const char *target, const char *pattern) {
    // compile regex
    int ret;
    regex_t rg;

    ret = regcomp(&rg, pattern, REG_ICASE);
    if (ret != 0) {
        fprintf(stderr, "failed compiling regex patter: %s err: %s", pattern, get_regerror(ret, &rg));
        ret = -1;
        goto out;
    }

    // match
    ret = regexec(&rg, target, 0, NULL, 0);
    if (ret == REG_NOMATCH) {
        fprintf(stderr, "target: %s not match with pattern: %s\n", target, pattern);
        ret = 0;
    } else if (ret == 0) {
        ret = 1;
    }else {
       ret = -1;
       fprintf(stderr, "regexec failed. pattern: %s err: %s", pattern, get_regerror(ret, &rg));
    }
    goto out;

out:
   if (&rg != NULL) {
     regfree(&rg);
   }
   return ret;
}


char*
get_regerror(int errcode, regex_t *rg) {
    size_t len = regerror(errcode, rg, NULL, 0);
    char *rgerror_buf = (char *)malloc(len *sizeof(char*));
    if (rgerror_buf == NULL) {
        fprintf(stderr, "failed allocating buffer for regerror\n");
        return "INVALID_ERR_ALLOCATE";
    }
    (void) regerror(errcode, rg, rgerror_buf, len);
    return rgerror_buf;
}