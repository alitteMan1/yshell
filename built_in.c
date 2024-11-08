#include <unistd.h>
#include <stdlib.h>
#include "built_in.h"
#include <sys/types.h>
#include <signal.h>
#include <sys/stat.h>

const char* built_in_cmd[] = {"pwd", "cd","exit", "jobs" ,"bg", "fg", NULL};
//int hash[] = {1,2,0};
extern job_list * current_job;
extern int iscontinue;
void jobs(job_list *jb_list){
    int nums = 0;
   // job_list* head = (job_list*)malloc(sizeof(jb_list));
   // head->next = jb_list;
    job_list * p = jb_list;
   // job_list * q = jb_list;
    while(p->next){
        if(p->next->job.stat == 'd'){
            job_list* todelete = p;
            p->next = p->next->next;
            free(todelete);
            continue;
        }
        p=p->next;
        nums++;
        printf("[%d]\t%d\t%c\t\t",nums,p->job.pid,p->job.stat);
        printf("%s\n",p->job.cmd);
        
    }
}

int ch_job_stat(job_list* jb, __pid_t pid, char stat){
    job_list* p = jb;
    while(p->next){
        p = p->next;
        if(p->job.pid == pid){
            p->job.stat = stat;
            return 1;
        }           
    }
    return 0;

}


int pwd(char** argvs){
    char work_dir[1024];
    if(getcwd(work_dir,1024)==NULL){
        perror("pwd failure\n");
    }
    printf("%s\n",work_dir);
    return 0;
}

int cd(char ** argv){
    passwd_p pswd;
    pswd = getpwuid(getuid());
    if(argv[1] == NULL){  
        chdir(pswd->pw_dir);
    }
    else{
        if(chdir(argv[1]) == -1){
            printf("yshell: cd: %s: ",argv[1]);
            fflush(stdout);
            perror("");
        }
    }
    return 0;
}

int bg(char** argv){
    if(argv[1] == NULL){
        printf("yshell: bg: 当前无此任务\n");
        return 0;
    }
    printf("argv[1]:%s", argv[1]);
    int num = atoi(argv[1]);
    job_list* p = current_job;
    int count = 0;
    while(p->next){
        count++;
        p=p->next;
        if(count == num){
            kill(p->job.pid, SIGCONT);
         
            // if(-1 == setpgid(p->job.pid, p->job.pid))
            //    perror("setpid");
          //  setp
         //   printf("set success\n");
            return 1;
        }
    }
    return 0;

}

int fg(char** argv){
    if(argv[1] == NULL){
        printf("yshell: fg: 当前无此任务\n");
        return 0;
    }
    printf("argv[1]:%s", argv[1]);
    int num = atoi(argv[1]);
    job_list* p = current_job;
    int count = 0;
    while(p->next){
        count++;
        p=p->next;
        if(count == num){
            kill(p->job.pid, SIGCONT);
            if(-1==tcsetpgrp(STDIN_FILENO,p->job.pid)){
                perror("tcset..");
                iscontinue = 1;
            }

            printf("qiantai jincheng :%d\n",tcgetpgrp(STDIN_FILENO));
            // if(-1 == setpgid(p->job.pid, p->job.pid))
            //    perror("setpid");
          //  setp
         //   printf("set success\n");
            return 1;
        }
    }
}

int parse_builtin_cmd(char ** argv){
    int i = 0;
    while(built_in_cmd[i] != NULL){
        if(strcmp(built_in_cmd[i],argv[0]) == 0)
            break;
        i++;
    }

    if(built_in_cmd[i] == NULL){
        return 0;
    }
    else{
        switch (i){
        case 0:   pwd(argv);  iscontinue = 1;               break;
        case 1:   cd(argv);   iscontinue = 1;                break;
        case 2:   exit(0);   iscontinue = 1;                break;
        case 3:   jobs(current_job);    iscontinue = 1;     break;
        case 4:   bg(argv);   iscontinue = 1;   printf("bg continue\n");            break;
        case 5:   fg(argv);                                   break;
        default:                                                  break;
        }
        return 1;
    }
}

