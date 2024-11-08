
#define _POSIX_C_SOURCE 200809L
#define _XOPEN_SOURCE 700

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <fcntl.h>
#include <sys/types.h>
#include <signal.h>
#include <sys/wait.h>
//#include <sys/sem.h>  
//#include <asm/signal.h>
#include "built_in.h"
//#define DEBUG
#define MAX_ARGS 40
#define MAX_CMD_NUM 10

/******************************
 * TODO: 管道 输入输出重定向  中断  jobs  
 * 
 * @author:lity
 * 
 */

job_list *current_job;
int redirc_fp;
int current_cmd_nums;
int iscontinue = 0;

int promt_and_get_input(const char* promt, char **line, size_t* len);
int parse_cmd(const char * cmd, int num, int(*pipes)[2]);
int get_nums(const char* line);
void split_cmd(const char *cmd, char **argv);
__pid_t exec_program(char** argv, int num, int(*pipes)[2]);
int judge_redir(char** argv, int nums);
int split_line_by_pipe(const char *line,char* cmd[]);
int check_null_cmd(char* cmd);


//SIGINT,SINGTTOU处理信号
void int_sigin_out(int sig){
    switch (sig){
    case SIGTTIN:
        printf("SIGTTIN\n");
        signal(SIGTTIN, int_sigin_out);
        break;
    case SIGTTOU:
        printf("SIGTTOU\n");
        signal(SIGTTOU, int_sigin_out);
        break;
    default:
        break;
    }
    
}

//中断处理信号 ctrl c
void int_res(int sig){
    char work_dir[1024];
    if (getcwd(work_dir,1024) == NULL){
        strcpy(work_dir,"-.-");
    }
    printf("\n");
    printf("\033[1;35myshell:\033[36m%s\033[0m$ ",work_dir);
    fflush(stdout);
    if(-1==signal(SIGINT,int_res)){
        perror("signal error\n");
    }
    printf("int:%d\n",sig);
}

void ctrlz(int sig){
    printf("ctrl z\n");
    signal(SIGTSTP, ctrlz);
}

    #define ASD (void(*)())-1

void sigcont(int sig){
    printf("continue\n");
    signal(SIGCONT,sigcont);
}

void sigchild(int sig){
     printf("parents recives a SIGCHLD signal\n");
    //  if(-1 == tcsetpgrp(STDIN_FILENO,getpid())) {
    //     perror("tc set failuer");
    //    }
    //     else{
    //         printf("pid:%d,gid:%d\n",getpid(),getgid());
    //    }
    int status;
    int waitid;
    while((waitid = waitpid(-1,&status,WNOHANG | WUNTRACED | WCONTINUED)) > 0){
        printf("sigchil wait id:%d groupid:%d\n",waitid, __getpgid(waitid));
        if(WIFEXITED(status)){
            iscontinue = 1;   
            printf("exit normally:");
            printf("pid:%d,gpid:%d\n",getpid(),__getpgid(waitid));
            tcsetpgrp(STDIN_FILENO,getpid());
        
        }else  if(WIFSIGNALED(status)){
            printf("exit for uncaptured signal\n");
            tcsetpgrp(STDIN_FILENO,getpid());
            iscontinue = 1;
        }else if(WIFSTOPPED(status)){
            printf("stoped :%d\n",WSTOPSIG(status));
            tcsetpgrp(STDIN_FILENO,getpid());
            iscontinue = 1;
        }else if(WIFCONTINUED(status)){
            printf("SIGCONTTTTTTTT\n");
            // if(tcsetpgrp(STDIN_FILENO,getpid())== -1)   {
            //     perror("tcsetpgrp error");
            // }
          //  iscontinue = 0;
        }else{
            printf("unknow status:%d\n",status);
        }
    }

 
    if(signal(SIGCHLD, sigchild) == SIG_ERR){
        perror("signal  child error\n");
    }
      printf("waitpid:%d\n",waitid);
//   if(WIFCONTINUED(status))        
//         printf("SIGCONTTTTTTTT\n");

   return ;
}
/* link statou */ 
int main(){
    signal(SIGTTIN, SIG_IGN);
    signal(SIGTTOU, SIG_IGN);               
    current_job = (job_list*)malloc(sizeof(job_list));
    if(signal(SIGINT,int_res) == SIG_ERR){
        perror("SIGINT!");
    }

    if(signal(SIGTSTP, ctrlz)==SIG_ERR){
          perror("SIGTERM!");
    }
    if(signal(SIGCONT, sigcont)==SIG_ERR){
          perror("SIGCONT!");
    }
     if(signal(SIGCHLD, sigchild)==SIG_ERR){
          perror("SIGCHID!");
    }
    char* line = NULL;
    size_t len;
    while(promt_and_get_input("yshell:", &line, &len) > 0){
        /**
         * 判断管线
         * 根据管线分割命令
         * 创建管道
         * 依次执行命令
         * 
         * 
         */
      //      signal(1,);
        iscontinue = 0;
        char *cmd[MAX_CMD_NUM];
        current_cmd_nums = split_line_by_pipe(line,cmd);
        if(current_cmd_nums == -1)  continue;
       
        int (*pipes)[2] = NULL;
        if(current_cmd_nums > 1){
            pipes = (int(*)[2])malloc(sizeof(int[2]) * (current_cmd_nums-1));
            for(int i = 0;i<current_cmd_nums-1;i++){
                pipe(pipes[i]);
                 #ifdef DEBUG
                printf("peipe1 %d pipe2 %d\n",pipes[i][0],pipes[i][1]);
                  #endif
            }
        }

        for(int i = 0; i < current_cmd_nums; i++){
            #ifdef DEBUG
            printf("comd%d\n",i);
            #endif
            parse_cmd(cmd[i], i, pipes);
            //printf("lslslsl\n");
           
        }
       
        if(pipes)   {   free(pipes); pipes = NULL;  }
        if(line)    {   free(line); line=NULL;      }
        current_cmd_nums = 0;
      
    }
    return 0;
}

/**
 * promt_and_get_input()
 * 从终端接受命令并打印shell提示
 */
int promt_and_get_input(const char* promt, char **line, size_t* len){
    char work_dir[1024];
    if (getcwd(work_dir,1024) == NULL){
        strcpy(work_dir,"-.-");
    }
    printf("\033[1;35m%s\033[36m%s\033[0m$ ",promt,work_dir);
    return getline(line, len, stdin);
}


int parse_cmd(const char * cmd, int num, int(*pipes)[2]){
    int nums = get_nums(cmd) + 1;
    if(nums == 1)      
        return -1;
   // printf("nums:%d\n",nums);
    //char** argv = (char**)malloc(sizeof(char*) * nums);
    char* argv[MAX_ARGS];
    split_cmd(cmd,argv);
    argv[nums-1] = NULL; 
    // TODO: 判断是否重定向  
    int redir_status = judge_redir(argv,nums);
    // for(int i = 0;i< nums;i++){
    //     if(argv[i]){
    //         printf("argv[%d]%s\n",i,argv[i]);
    //     }
    // }

    if(redir_status == -1)
        return -1;
    if(!parse_builtin_cmd(argv)){      
         pid_t cpid = exec_program(argv,num,pipes);
         
        
         job_list * newnode = (job_list*)malloc(sizeof(job_list));
         if(!newnode){
            printf("newnode alloc faliure\n");
         }
        newnode->job.pid=cpid;
        newnode->job.stat = 'r';
        newnode->job.cmd = cmd;
        newnode->next = NULL;
       // EACCES
            if(-1 == setpgid(newnode->job.pid,0))
               perror("setpid");
            else{
                //printf("success\n");
                if(-1 == tcsetpgrp(STDIN_FILENO,cpid)){
                    perror("set tc error");
                }
               }

        // if(!current_job){
        //     current_job = newnode;
        // }
        // else{
        //     job_list *p = current_job; 
        //     while(p->next){
        //         p = p->next;
        //     }
        //     p->next = newnode;

        // }
         job_list *p = current_job; 
            while(p->next){
                p = p->next;
            }
            p->next = newnode;
         if(pipes){
            if(num>0)
                close(pipes[num-1][0]);
            if(num<current_cmd_nums-1){
               // printf("close pipes[%d][1]",num);
                close(pipes[num][1]);
            }
         }
            //close(pipes[0][1]);
        //close(pipes[0][0]);
        // wait(NULL);
        
       
        // printf("main process wait id:%d groupid:%d\n",waitid, __getpgid(waitid));

        jobs(current_job);
    }
    printf("iscontinue:%d\n",iscontinue);
    while(!iscontinue){
            
        }
    
    if(redir_status){
        dup2(redir_status,1);
        close(redirc_fp);
    }

    for(int i =0;i<nums-1;i++){
        if(!argv[i]){
            free(argv[i]);
            argv[i]=NULL;
        }
    }
    return 0;
}

int get_nums(const char* line){
    int nums=0;
    const char* c = line;
    int status = 1;
    while(*c != '\0'){ 
        if(*c == ' '){
            status = 1;
        }
        else{
            if(*c!= '\n' && status == 1){
                nums++;
                status = 0;
            }
        }
        c++;
    }
    return nums;
}
void split_cmd(const char *cmd, char **argv){
    const char* c = cmd;
    int num = 0;
    
    while(*c != '\0'){
  
        while(*c == ' ' || *c == '\n'){
            c++;
        }   
        if(*c=='\0'){
            break;
        }
        int i = 0;      
        while(*(c+i) != ' ' && *(c+i) !='\n' && *(c+i) !='\0'){
            i++;
        }
     //   printf("args:%d num:%d\n",num,i);
        argv[num] = (char*)malloc(sizeof(char)*(i+1));
        memcpy(argv[num],c,sizeof(char)*i);
        argv[num][i]='\0';
        num++;
        c+=i+1;
    }
    argv[num]=NULL;
  //  printf("argv%d=NULL",num);

    return;

}

__pid_t exec_program(char** argv, int num , int(*pipes)[2]){
    __pid_t child = fork();
    if(child){              //parent 
        
        if(child < 0){
            printf("fork failure\n");
            exit(0);
        }
        else{
            printf("parent pid:%d, gpid %d\n", getpid(),getpgrp());
            return child;
        }
    }
  
    else{                   //child 
        // int out_bak = dup(1);
        // int fp = open("log.txt",O_CREAT | O_WRONLY | O_TRUNC, 0666);
        // printf("std:%d\n",fp);
        // if(fp<0){
        //     perror("open error!\n");
        // }
        // dup2(fp,1);
        // printf("fp:%d\n",fp);
        // dup2(out_bak,1);
        // printf("out_bak:%d\n",fp);
       
        //pipes
        
        printf("child pid:%d, gpid %d\n", getpid(),getpgrp());
            if(pipes != NULL){
                if(num>0)
                    dup2(pipes[num-1][0],0);
                if(num<current_cmd_nums-1)
                    dup2(pipes[num][1],1);
            }
        if(execvp(argv[0],argv) == -1){        //failure -> kill child
            printf("%s: 未找到命令\n",argv[0]);
            exit(0);
        }
        return 0;
    }
}


int judge_redir(char** argv, int nums){
    int i = 0;

    while(argv[i] != NULL){
    
        if(strchr(argv[i],'>')){
            if(i == 0 || argv[i+1] == NULL){
                perror(" > error");
                return -1;
            }
            else{
                int out_bak = dup(1);
                redirc_fp = open(argv[i+1],O_CREAT | O_WRONLY | O_TRUNC, 0666);
                if(redirc_fp < 0){
                    perror("open file error");
                    return -1;
                }
                if(dup2(redirc_fp,1) == -1){
                    printf("can't redirect fp!\n");
                    return -1;
                }
                free(argv[i]);
                argv[i] = NULL;
                return out_bak;
            }
        }
        i++;
    }
    return 0;
}

int split_line_by_pipe(const char *line,char* cmd[]){
    int cmd_num = 0;
    int i = 0;
    char *c = line;
    while(*c != '\0'){
       // printf("begin i:%d\n",i);
        while(*(c+i) != '|' && *(c+i) != '\n'){
            i++;
        }
        cmd[cmd_num] = (char*)malloc(sizeof(char) * (i+1));
        memset(cmd[cmd_num],0,i+1);
        memcpy(cmd[cmd_num],c,i);
        #ifdef DEBUG
        printf("cmd[%d]:%s\n",cmd_num,cmd[cmd_num]);
        #endif
        cmd[cmd_num][i] = '\0';
        c=c+i+1;
        cmd_num++;
        i=0;
    }
    
    if(cmd_num > 1){
        for(int j = 0;j < cmd_num; j++){
            if(check_null_cmd(cmd[j])){
                //printf("cmd %d : is null\n",j );
                return -1;
            }
            // else{
            //     printf("happy\n");
            // }
        }
    }
    return cmd_num;

}

int check_null_cmd(char* cmd){
    char* c = cmd;
    while(*c != '\0'){
        if(*c != ' ')
            return 0;
        c++;
    }
    return 1;
}

