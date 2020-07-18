/*******************************************************************************
* Name        : minishell.c
* Author      : Akhilesh Reddy worked with Avaneesh Kolluri.
* Date        : 4/3/2020
* Description : Minishell Assignment
* Pledge      : I pledge my honor that I have abided by the Stevens Honor System.
******************************************************************************/
#include <signal.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>
#include <sys/wait.h>
#include <ctype.h>
#include <wait.h>
#include <setjmp.h>
#include <sys/types.h>
#include <pwd.h>

#define BRIGHTBLUE "\x1b[34;1m"
#define DEFAULT    "\x1b[0m"

volatile sig_atomic_t t = 0;
volatile sig_atomic_t is_child = false;
sigjmp_buf jmpbuf;

void catch_signal(int sig) {
    //t = 1;
    
    if(!is_child){
        write(STDOUT_FILENO, "\n", 1);
    }
    t = sig;
    siglongjmp(jmpbuf, 1);
}

int main(int argc, char *argv[]) {
    
    
    //bool e = true;
    

    char **args;
    int count = 0;

    struct sigaction action;
    memset(&action, 0, sizeof(struct sigaction));
    action.sa_handler = catch_signal;
    action.sa_flags = SA_RESTART;
    if (sigaction(SIGINT, &action, NULL) == -1) {
        fprintf(stderr, "Error: Cannot register signal handler. %s.\n", strerror(errno));
        return EXIT_FAILURE;
    }
    
    sigsetjmp(jmpbuf, 1);
    
    while(true){
        //SIGJMP back here
        
        if (t == SIGINT){
            if (is_child){
                
                for(int i=0; i<count;i++){
                        free(args[i]);
                    }
                    free(args);
            }
            is_child = false;
            t = 0;
        }
        count = 0;
        char buf[8192];
        char input[8192];
        //char * input = (char *)malloc((8192) * sizeof(char ));
        ssize_t bytes_read;
        if(getcwd(buf, sizeof(buf)) == NULL){
            fprintf(stderr, "Error: Cannot get current working directory. %s.\n", strerror(errno));
            return EXIT_FAILURE;
        }
        printf("[%s%s%s]$ ",BRIGHTBLUE, buf, DEFAULT);
        fflush(stdout);
        
        if((bytes_read = read(STDIN_FILENO, input, sizeof(input)-1)) < 0){
            return EXIT_FAILURE;
        }
        
        //Null terminate
        input[bytes_read] = '\0';
        
        //printf("NUMBER BYTES %ld\n",bytes_read);
        if (bytes_read == 1){
            //printf("IN HERE\n");
            continue;
        }

//        if (bytes_read == 0){
//            //printf("IN HERE\n");
//            write(STDIN_FILENO,"\n",1);
//            return EXIT_SUCCESS;
//        }
        
        char *temp;
        
        if(( args = (char **)malloc((2048) * sizeof(char *)) ) == NULL){
            fprintf(stderr, "Error: malloc() failed. %s.\n", strerror(errno));
            return EXIT_FAILURE;
        }
        temp = (char *)strtok(input, " ");
        int i=0;

        while(temp != NULL){
            if ((args[i] = (char *)malloc((strlen(temp)+1) * sizeof(char))) == NULL){
                fprintf(stderr, "Error: malloc() failed. %s.\n", strerror(errno));
                return EXIT_FAILURE;
            }
            strcpy(args[i],temp);
            char *p;
            if((p = strchr(args[i], '\n')) != NULL){
                *p = '\0';
            }
            temp = (char *)strtok(NULL, " ");
            i++;
            count++;
        }

         if(args[0][0] == '\0'){
             //printf("HERE");
             for(int i=0; i<count;i++){
                  free(args[i]);
           }
             free(args);
             continue;

         }

        args[i] = NULL;
        
        int l = 0;
        while(args[l] != NULL){
            if(args[l][0] == '\0'){
                free(args[l]);
                args[l] = NULL;
            }
            l++;
        }
        
    
        if(args[0] == NULL){
            //printf("HERE");
            for(int i=0; i<count;i++){
                free(args[i]);
            }
            free(args);
            continue;
        }
        
        if(((strcmp(args[0],"cd\"\"")) == 0) && (args[1] == NULL)){{
            uid_t user = getuid();
            struct passwd *p;
            
            if((p = getpwuid(user)) == NULL){
                fprintf(stderr, "Error: Cannot get passwd entry. %s.\n", strerror(errno));
            }
            char *home = p->pw_dir;
            //Check return value if it worked, if it didnt print out what error messages !!!!!
            if(chdir(home) == -1){
                fprintf(stderr, "Error: Cannot change directory to '%s'. %s.\n", args[1], strerror(errno));
            }
        }
            
        } else if((strcmp(args[0],"exit")) == 0){
            //printf("Before Exit Success!\n");
            for(int i=0; i<count;i++){
                free(args[i]);
            }
            free(args);
            return EXIT_SUCCESS;
            //break;
            
        } else if((strcmp(args[0],"cd")) == 0){
            //printf("args[0]: %s\n", args[0]);
            //printf("%d\n",i);
            
            
            if(args[1] == NULL || ((strcmp(args[1], "~") == 0) && ((args[2] == NULL) || (args[2][0] == '\0') ) ) || args[1][0] == '\0'){ // or if we get a tilda
                //do something
                uid_t user = getuid();
                struct passwd *p;
                
                if((p = getpwuid(user)) == NULL){
                    fprintf(stderr, "Error: Cannot get passwd entry. %s.\n", strerror(errno));
                }
                char *home = p->pw_dir;
                //Check return value if it worked, if it didnt print out what error messages !!!!!
                if(chdir(home) == -1){
                    fprintf(stderr, "Error: Cannot change directory to '%s'. %s.\n", args[1], strerror(errno));
                }
                
            }else{
                //Check if too many arguments were given
                bool sp = false;
                if(args[2] != NULL && (args[2][0] != '\0')){
                    sp = true;
                }
            
                if(!sp){
                    //CHECK IF CHDIR FAILS
                    
                    if (strncmp(args[1],"~/",2) == 0){
                        uid_t user = getuid();
                        struct passwd *p;
                        
                        if((p = getpwuid(user)) == NULL){
                            fprintf(stderr, "Error: Cannot get passwd entry. %s.\n", strerror(errno));
                        }
                        char *home = p->pw_dir;
                        //Check return value if it worked, if it didnt print out what error messages !!!!!
                        //printf("Home: %s", home);

                        //const char tilda[10] = "~/";
                        char *ret;
                        ret = strstr(args[1],"~/");
                        //printf("args[1]:%s\n",args[1]);
                        ret++;
                        ret++;
                        //printf("ret:%s\n",ret);
                        strcat(home,"/");
                        strcat(home,ret);
                        //printf("Home: %s", home);
                        if (chdir(home) == -1){
                            fprintf(stderr, "Error: Cannot change directory to '%s'. %s.\n", args[1], strerror(errno));
                        }
                    } else if(chdir(args[1]) == -1){
                        //printf("here\n");
                        fprintf(stderr, "Error: Cannot change directory to '%s'. %s.\n", args[1], strerror(errno));
                        
                    }
                } else{
                    fprintf(stderr, "Error: Too many arguments to cd.\n");
                }
                
            }
            
            
        } else{
            pid_t pid;
            
            if ((pid = fork()) < 0){
                fprintf(stderr, "Error: fork() failed. %s.\n", strerror(errno));
                for(int i=0; i<count;i++){
                    free(args[i]);
                }
                free(args);
                return EXIT_FAILURE;
            }
            else if(pid == 0){

                if(execvp(args[0], args) < 0){
                    fprintf(stderr, "Error: exec() failed. %s.\n", strerror(errno));
                    
                    exit(EXIT_FAILURE);
                }

                
            } else{
                is_child = true;
                int status;
                if(waitpid(pid, &status, 0) < 0){
                    fprintf(stderr, "Error: wait() failed. %s.\n", strerror(errno));
                    for(int i=0; i<count;i++){
                        free(args[i]);
                    }
                    free(args);
                }
                is_child = false;
            }
        }
       // printf("%d\n",count);
        for(int i=0; i<count;i++){
            free(args[i]);
        }
        free(args);
        memset(input,0, 8192);
     
    }

    return EXIT_SUCCESS;
}
