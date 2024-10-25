#include <unistd.h>
#include "built_in.h"

const char* built_in_cmd[] = {"pwd", "cd", NULL};
//int hash[] = {1,2,0};

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
        case 0:   pwd(argv);    break;
        case 1:   cd(argv);     break;
        default:                break;
        }
        return 1;
    }
}

