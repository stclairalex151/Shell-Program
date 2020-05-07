/*
 * Program name: 3207project2
 * Programmer:   Alex St.Clair
 * Program Desc: Shell program that handles internal and external commands
 */

#include <unistd.h>
#include <dirent.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <fcntl.h>
#define DEBUG 0

typedef struct{
    int input;  //flag for input redirection
    int output; //flag for output redirection
    int background; //flag for background execution
    int pipe;   //flag for pipe operator
}Flags; //flag struct

typedef struct{
    char* cwd;//current working directory
    char* user;
    char* name;
}Environ;   //environment struct

void setFlags(char ** parsed, Flags *f);
char ** parse(char* cmd);
int getType(char ** command, char* cwd);
void execInternal(char ** command, Environ * environ, Flags f);
void runFile(char ** command , char * cwd , Flags f);
void runSys(char ** command, Flags f);

int main(int argc, char** argv) {
    Environ environ;    //environment struct
    Flags f;            //flags struct
    Flags *fptr = &f;   //pointer to flag struct
    char command[500];  //buffer for input string
    char cwdstring[200];//buffer for cwd
    char ** parsed;
    int hasBatch = 0;   //flag for batch mode (0 is user, 1 is batch)
    FILE *batch;        //file pointer for batch (only is declared when argc>1
    
    environ.user = "alexstclair";
    environ.name = "3207project2.c";
    getcwd(cwdstring, sizeof(cwdstring));//gets current dir
    environ.cwd = cwdstring;
    
    if(argc > 1){  //if program was executed with a batch file
        hasBatch = 1;
        char* filename = argv[1];
        batch = fopen(filename , "r");
    }
    
    while(1){
        f.background = 0;
        f.input = 0;
        f.output = 0;   //reset flags for next command
        f.pipe = 0;

        fflush(stdin);
        
        if(hasBatch){
            fgets(command, sizeof(command), batch);
            if(!strcmp(command, "exit"))
                exit(0);
        }else{
            printf("%s> " , environ.name);//print prompt
            fgets(command, 500, stdin);
        }
        
        command[strlen(command) - 1] = '\0';//trims the newline off the command

        parsed = parse(command);            //break string into tokens
        setFlags(parsed , fptr);

        int type = getType(&parsed[0], environ.cwd);//1 is internal, 2 is in cwd, 3 is extern

//printf("i: %d\no: %d\nb: %d\np: %d\n" , fptr->input,fptr->output,fptr->background,fptr->pipe);

        if(type == 1)
            execInternal(parsed , &environ , f);
        else if(type == 2)
            runFile(parsed, environ.cwd , f);
        else
            runSys(parsed, f);
    }
    return(0);
}

/**
 * Breaks down input string to tokens
 * @param cmd input string
 * @return array of character pointers
 */
char ** parse(char* cmd){
    char** parsed;   //the array of strings
    parsed = (char**) malloc(sizeof(char) * 500);    //10 words, 50 char words
    char* token = strtok(cmd, " "); //sets the first token
    int i = 0;  //iterator
    
    while (token){ 
        #if DEBUG
            printf("\nelement %d is |%s|", i , token); 
        #endif
        parsed[i] = (char*) token;//insert token 
       
        i++;
        token = strtok(NULL, " ");//sets token to next word 
    }
    parsed[i] = NULL;//sets the end string
    
    #if DEBUG
        printf("\nsize of parsed is %d\n" , i);  
    #endif
    
    return parsed;
}

/**
 * Sets appropriate flags for command
 * @param parsed input string
 * @param f flags struct to be modified
 */
void setFlags(char ** parsed, Flags *f){
    int i = 0;
    while(parsed[i] != NULL){
        //set applicable flags
        if(!strcmp(parsed[i] , "<"))
            f->input = 1;
        else if(!strcmp(parsed[i] , ">"))
            f->output = 1;
        else if(!strcmp(parsed[i] , "&"))
            f->background = 1;
        else if(!strcmp(parsed[i] , "|"))
            f->pipe = 1;
        i++;
    }
}

/**
 * Checks to see if cmd is internal (1), 
 * exists in cwd(2), or is external (3)
 * @param command input string
 * @param cwd current working directory
 * @return typecode as specified above
 */
int getType(char ** command , char* cwd){      
    int ret;//return value
    struct stat buffer; //status buffer
    char* fd;   //hypothetical path of file in cwd
    strlcpy(fd, cwd, 100);
    fd = strncat(fd, "/" , 50); //appends file to cwd
    fd = strncat(fd, command[0], 50);

    if (!strcmp(command[0], "cd"))//if cmd is cd
        ret = 1;
    else if(!strcmp(command[0], "clear"))//if cmd is clr
        ret = 1;
    else if(!strcmp(command[0], "ls"))//if cmd is ls
        ret = 1;
    else if(!strcmp(command[0], "environ"))//if cmd is environ
        ret = 1;
    else if(!strcmp(command[0], "echo"))//if cmd is echo
        ret = 1;
    else if(!strcmp(command[0], "help"))//if cmd is help
        ret = 1;
    else if(!strcmp(command[0], "pause"))//if cmd is pause
        ret = 1;
    else if(!strcmp(command[0], "exit"))//if cmd is quit
        ret = 1;
    else if(stat(fd, &buffer) == 0)//check to see if cmd is name of file in cwd
        ret = 2;
    else
        ret = 3;
    return ret;
}

/**
 * handles execution of any internal command
 * @param command input string
 * @param environ environment struct
 */
void execInternal(char ** command, Environ * environ , Flags f){
    if (!strcmp(command[0], "cd")){//if cmd is cd
        
        if(strlen(command[1]) == 0)  //command has no args, print cwd
            printf("CWD is:\n%s\n" , environ->cwd);
        
        else{//command[1] should be appended to cwd
            char buffer[100];//buffer for the new cwd
            int val = chdir(command[1]);
            if(val != 0){
                printf("%s: no such file or directory.\n" , command[1]);
                return;
            }
            getcwd(buffer, 100);
            printf("\ncwd is now: %s\n" , buffer);
            environ->cwd = buffer;
        }
    }
    
    else if(!strcmp(command[0], "clear")){//if cmd is clr
        printf("\033[H\033[2J");
    }
    
    else if(!strcmp(command[0], "ls")){//if cmd is ls
        struct dirent *de;  //directory entry
        DIR * ptr = opendir(".");//ptr to directory
        if(ptr == NULL){
            printf("\nDirectory not found.");
            return;
        }
        
        int saved_stdout;
        if(f.output){   //set up stdout for output redirection
            int i = 0;
            while( strcmp(command[i], ">") != 0)
                i++;
            i++;
            char* filename = command[i];//filename is token after > symbol

            saved_stdout = dup(1);  //dup stdout to restore later
            int file_desc = open(filename, O_RDWR|O_CREAT,S_IRUSR|S_IWUSR);//open the file
            dup2(file_desc, 1);//filename is now fd1
            close(file_desc); //close the duplicate
        }
        
        while((de = readdir(ptr)) != NULL)
            printf("%s\n" , de->d_name); //print the directory name
        printf("\n");
        closedir(ptr);
        
        if(f.output){
            dup2(saved_stdout, 1);  //restore fd1 with copy
            close(saved_stdout);    //close old fd
        }
    }
    
    else if(!strcmp(command[0], "environ")){//if cmd is environ
        int saved_stdout;
        if(f.output){   //set up stdout for output redirection
            int i = 0;
            while( strcmp(command[i], ">") != 0)
                i++;
            i++;
            char* filename = command[i];//filename is token after > symbol

            saved_stdout = dup(1);  //dup stdout to restore later
            int file_desc = open(filename, O_RDWR|O_CREAT,S_IRUSR|S_IWUSR);//open the file
            dup2(file_desc, 1);//filename is now fd1
            close(file_desc); //close the duplicate
        }
        printf("Shell name: %s\nUser: %s\ncwd: %s\n\n",environ->name, environ->user, environ->cwd);
        
        if(f.output){//switch stdout back 
            dup2(saved_stdout, 1);  //restore fd1 with copy
            close(saved_stdout);    //close old fd
        }
    }
    
    else if(!strcmp(command[0], "echo")){//if cmd is echo
        int saved_stdout;
        if(f.output){   //set up stdout for output redirection
            int i = 0;
            while( strcmp(command[i], ">") != 0)
                i++;
            i++;
            char* filename = command[i];//filename is token after > symbol

            saved_stdout = dup(1);  //dup stdout to restore later
            int file_desc = open(filename, O_RDWR|O_CREAT,S_IRUSR|S_IWUSR);//open the file
            dup2(file_desc, 1);//filename is now fd1
            close(file_desc); //close the duplicate
        }
        int i = 1;
        while(strcmp(command[i], "\0") != 0){
            printf("%s " , command[i]);
            i++;
        }
        printf("\n");
        
        if(f.output){//switch stdout back 
            dup2(saved_stdout, 1);  //restore fd1 with copy
            close(saved_stdout);    //close old fd
        }
    }
    
    else if(!strcmp(command[0], "help")){//if cmd is help
        int saved_stdout;
        if(f.output){   //set up stdout for output redirection
            int i = 0;
            while( strcmp(command[i], ">") != 0)
                i++;
            i++;
            char* filename = command[i];//filename is token after > symbol

            saved_stdout = dup(1);  //dup stdout to restore later
            int file_desc = open(filename, O_RDWR|O_CREAT,S_IRUSR|S_IWUSR);//open the file
            dup2(file_desc, 1);//filename is now fd1
            close(file_desc); //close the duplicate
        }
        
        FILE *fp = fopen("readme2.txt" , "r");
        if(fp == NULL){
            printf("\nError opening file.");
            return;
        }
        char ch;
        while((ch = fgetc(fp)) != EOF){
            printf("%c" , ch);
        }
        fclose(fp);
        
        if(f.output){//switch stdout back 
            dup2(saved_stdout, 1);  //restore fd1 with copy
            close(saved_stdout);    //close old fd
        }
    }
    
    else if(!strcmp(command[0], "pause")){//if cmd is pause
        printf("%s", "\nPress enter to continue...");
        getchar();
    }
    
    else    //cmd is quit
        exit(0);
}

/**
 * executes a program that exists in cwd
 * @param command the line of input
 * @param cwd the current working directory
 */
void runFile(char ** command , char * cwd, Flags f){
    char * name = command[0];//first word in command string
    char * path = cwd;       //new path that includes filename
    struct stat buffer;      //status buffer for accessing file
    
    strlcat(path, "/" , 100);
    strlcat(path, name , 100);  //add "/filename" to the file path
    
    if(stat(path, &buffer)){//returns -1 on failure
        printf("\nFile cannot be run, is not in specified path.");
        return;
    }
    
    //check for I/O redirection
    int saved_stdout;
    if(f.output){   //set up stdout for output redirection
        int i = 0;
        while( strcmp(command[i], ">") != 0)
            i++;
        i++;
        char* filename = command[i];//filename is token after > symbol

        saved_stdout = dup(1);  //dup stdout to restore later
        int file_desc = open(filename, O_RDWR|O_CREAT,S_IRUSR|S_IWUSR);//open the file
        dup2(file_desc, 1);//filename is now fd1
        close(file_desc); //close the duplicate
    }
    int saved_stdin;
    if(f.input){   //set up stdout for output redirection
        int i = 0;
        while( strcmp(command[i], "<") != 0)
            i++;
        i++;
        char* filename = command[i];//filename is token after > symbol

        saved_stdin = dup(0);  //dup stdout to restore later
        int file_desc = open(filename, O_RDWR|O_CREAT,S_IRUSR|S_IWUSR);//open the file
        dup2(file_desc, 0);//filename is now fd1
        close(file_desc); //close the duplicate
    }
    if(f.pipe){
        int e = 0;
        while(strcmp(command[e], "\0") != 0){
            printf("command[%d]: %s\n" , e, command[e]);
            e++;
        }
    }
    else{//no pipe
        int pid = fork();   //forks process
        if(pid < 0){
            printf("\nfork failed. exiting.");
            exit(1);
        }
        else if(pid == 0){   //child
            execvp(path, command);

            //should never enter here
            printf("\nChild could not execute file.");
            exit(1);
        }
        else{//parent
            //wait for file to finish executing
            int status = 0;
            wait(&status);
            printf("\nchild exited with status of %d\n" , status);
        }
    }
    
    //Restore file descriptors
    if(f.output){//switch stdout back 
        dup2(saved_stdout, 1);  //restore fd1 with copy
        close(saved_stdout);    //close old fd
    }
    if(f.input){//switch stdout back 
        dup2(saved_stdin, 0);  //restore fd1 with copy
        close(saved_stdin);    //close old fd
    }
}

/**
 * Executes external command, if exists in /bin 
 * or /usr/bin
 * @param command the input string
 */
void runSys(char ** command, Flags f){
    //check for I/O redirection
    int saved_stdout;
    if(f.output){   //set up stdout for output redirection
        int i = 0;
        while( strcmp(command[i], ">") != 0)
            i++;
        i++;
        char* filename = command[i];//filename is token after > symbol

        saved_stdout = dup(1);  //dup stdout to restore later
        int file_desc = open(filename, O_RDWR|O_CREAT,S_IRUSR|S_IWUSR);//open the file
        dup2(file_desc, 1);//filename is now fd1
        close(file_desc); //close the duplicate
    }
    int saved_stdin;
    if(f.input){   //set up stdout for output redirection
        int i = 0;
        while( strcmp(command[i], "<") != 0)
            i++;
        i++;
        char* filename = command[i];//filename is token after > symbol

        saved_stdin = dup(0);  //dup stdout to restore later
        int file_desc = open(filename, O_RDWR|O_CREAT,S_IRUSR|S_IWUSR);//open the file
        dup2(file_desc, 0);//filename is now fd1
        close(file_desc); //close the duplicate
    }
    if(f.pipe){
        printf("got here.");
    }
    
    int pid = fork();   //forks process
    if(pid < 0){
        printf("\nfork failed. exiting.");
        exit(1);
    }
    else if(pid == 0){   //child
        if(f.background)
            sleep(10);
        execvp(command[0], command);
        printf("File could not be found in either search path.\n");
    }
    else{//parent
        //wait for file to finish executing
        int status = 0;
        if(f.background == 0)
            wait(&status);
        //printf("\nchild exited with status of %d\n" , status);
    }
    
    //Restore file descriptors
    if(f.output){//switch stdout back 
        dup2(saved_stdout, 1);  //restore fd1 with copy
        close(saved_stdout);    //close old fd
    }
    if(f.input){//switch stdout back 
        dup2(saved_stdin, 0);  //restore fd1 with copy
        close(saved_stdin);    //close old fd
    }
}