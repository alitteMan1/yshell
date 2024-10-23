#include <stdio.h>
#include <string.h>
#include <pwd.h>

#ifndef PASSWD_P
#define PASSWD_P
    typedef struct passwd* passwd_p;
#endif

int parse_builtin_cmd(char ** argvs);