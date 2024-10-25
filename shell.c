#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <fcntl.h>
#include <sys/wait.h>

#include "built_in.h"

/******************************
 * TODO: 管道 输入输出重定向  中断  jobs  
 * 
 * @author:lity
 * 
 * 
 * 
 * 
 * 
 * 
 * 
 * 
 * 
 * 
 * 
 */


int redirc_fp;

int promt_and_get_input(const char* promt, char **line, size_t* len);
int parse_cmd(const char * cmd, size_t len);
int get_nums(const char* line);
void split_cmd(const char *cmd, char **argv);
__pid_t exec_program(char** argv);
int judge_redir(char** argv, int nums);


/* shell 主函数 */ 
int main(){
    char* line = NULL;
    size_t len;
    while(promt_and_get_input("yshell:", &line, &len) > 0){
        //printf("cmd lin: %s\n",line);
        parse_cmd(line, len);
        free(line);
        line=NULL;
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


int parse_cmd(const char * cmd, size_t len){
    int nums = get_nums(cmd) + 1;
    if(nums == 1)      
        return -1;
   // printf("nums:%d\n",nums);
    char** argv = (char**)malloc(sizeof(char*) * nums);
    //char* argv[30];
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
         exec_program(argv); 
         wait(NULL);
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
    
    if(!argv){
        free(argv);
        argv=NULL;
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
        while(*(c+i) != ' ' && *(c+i) !='\n'){
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

__pid_t exec_program(char** argv){
    __pid_t child = fork();
    if(child){              //parent 
        if(child < 0){
            printf("fork failure\n");
            exit(0);
        }
        else{
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
