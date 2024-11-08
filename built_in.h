
#ifndef BUILT_IN
#define BUILT_IN

#include <stdio.h>
#include <string.h>
#include <pwd.h>

#ifndef PASSWD_P
#define PASSWD_P
    typedef struct passwd* passwd_p;
#endif



typedef struct jobinfo{
    __pid_t pid;
    char stat;
    char *cmd;
}job_info;



typedef struct joblist{
    job_info job;
    struct joblist* next;
}job_list;



int parse_builtin_cmd(char ** argvs);
void jobs(job_list *jb_list);
#endif