/* 
 * File:   main.c
 * Author: mulligan
 *
 * Created on September 30, 2016, 9:56 PM
 */

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <errno.h>
#include <string.h>
#include <sys/signal.h>
#include "singnal_handlers.c"

struct bgList *head = NULL; //Global variable;

/*
 * My Shell Project
 */
char* getCmd();
char** getArgs(char* command);
void printStats(pid_t pid);
int isBackground(char** arguments);
void childHandle (int signum);


int main(int argc, char** argv){
    
    //Signal handler
    struct bgList *head = NULL;
    signal(SIGINT,SIG_IGN);
    while(1){
        siginfo_t signalInfo;
        signalInfo.si_pid = 0;
        waitid(P_ALL, 0, &signalInfo, WEXITED | WSTOPPED | WNOWAIT |WNOHANG);
        if(signalInfo.si_pid != 0){ //terminated child found
            printStats(signalInfo.si_pid);
            waitpid(signalInfo.si_pid,NULL,0);
                
        }
        int isBG = 0;
        char* cmd = getCmd();
        if (cmd == NULL) return (EXIT_SUCCESS);
        char** args = getArgs(cmd);
        
        
        if (args[0] == NULL){
            printf("NO COMMAND\n");
            free(cmd);
            continue;
        } 

       //Internal Commands
       if (!strcmp("exit",args[0])){
            return (EXIT_SUCCESS);
       }
        isBG = isBackground(args);
        pid_t pid = fork();

        if (pid == 0){ //child
            if (execvp(args[0],args) == -1){
                printf("Error: %s\n", strerror(errno));
                exit(0);
            }
        } else if (isBG) { //parent and not a background process
            struct bgList *new = newBgNode(pid);
            addBgNode(new,head);
        } else {
            while (0 == waitid(P_ALL, 0, &signalInfo, WEXITED | WSTOPPED | WNOWAIT)){ //Loop through and clear out zombie processes
                printStats(signalInfo.si_pid);
                waitpid(signalInfo.si_pid,NULL,0);
            }
        }
        
        free(cmd);
        free(args);

    }
    
    return (EXIT_SUCCESS);
}

char* getCmd(){
    char *command;
    ssize_t bufSize = 0;
    printf("$");
    getline(&command,&bufSize,stdin);
    return command;
}

//Returns a string of single character arguments and adds an end of string \0
//to the end of the command
char** getArgs(char* command) {
    char** args = (char**) malloc(sizeof(char*)*(sizeof(command)/sizeof command[0])); //make char** able to hold max number of arguments for command size
    int argsSize = 0;
    int position = 0;
    
    while (command[position] != '\n'){ //run through command until end
        if (command[position] != ' '){ //if not white space
            args[argsSize] = &command[position]; //make pointer to first letter of string
            argsSize++;
            do { //run through string until end or whitespace
                position++;
            } while(command[position] != ' ' && command[position] != '\n');
            if (command[position] == '\n'){ //if end of command break from string 
                break;
            } else { // if white space replace with null char
                command[position] = '\0'; 
            }
        }
      
        position++;
    }
    command[position] = '\0'; //replace '\n' with null   

    return args;

}

void printStats(pid_t pid){
    char str[50]; //string for holding file name
    FILE * file;
    sprintf(str, "/proc/%d/stat", (int)pid); //put unique pid stat file to str
    file = fopen(str,"r");
    if (file == NULL){
        printf("Error Opening file");
        return;
    }
    int z;
    char c;
    unsigned long h, ut, st; //read file and store variables
    fscanf(file, "%d %s %c %d %d %d %d %d %u %lu %lu %lu %lu %lu %lu", 
            &pid, str, &c, &z, &z, &z, &z, &z,
	    (unsigned *)&z, &h, &h, &h, &h, &ut, &st);
    fclose(file); 
    //print stats
    printf("PID: %d \n", pid);
    printf("CMD: %s \n", str);
    printf("UT: %lf s \n", ut*1.0f/sysconf(_SC_CLK_TCK));
    printf("ST: %lf s\n", st*1.0f/sysconf(_SC_CLK_TCK));

}

int isBackground(char** arguments){
    int i = 0;
    while (arguments[i] != NULL){
        i++;
    }
    i--;
    if (arguments[i][0] == '&'){
        arguments[i] = NULL;
        return(1);
    } else {
        return(0);
    }
}

void childHandle (int signum){
    siginfo_t signalInfo;
    if (0 == waitid(P_ALL, 0, &signalInfo, WEXITED | WSTOPPED | WNOWAIT | WNOHANG)){ 
        if (-1 == removeBgNode(signalInfo.si_pid,head)){ //If not a BG process return
            return;
        }
        printStats(signalInfo.si_pid);
        waitpid(signalInfo.si_pid,NULL,0);
                
    }
}