#ifndef _POSIX_C_SOURCE
	#define  _POSIX_C_SOURCE 200809L
#endif

#include <stdio.h> //fprintf() realloc() free() exit() execvp()
#include <stdlib.h> //malloc() realloc() free() exit() execvp() EXIT_SUCCESS ,EXIT_FAILURE, atoi()
#include <string.h> //strcmp() strtok()

#include <sys/wait.h> //wait pid
#include <unistd.h> //chdir() fork() exec() pid_t

#include <sys/stat.h>     //dup2()
#include <sys/types.h>
#include <errno.h>
#include <fcntl.h>      //open() close()

#include <dirent.h> //FILE, DIR

char **lsh_split_line(char *line);
int cmdnum;     //for counting the number of one command
int status=1;   //for recording the status
int inPos=0;    //for recording the position of "<"
int outPos=0;   //for recording the position of ">"

char cmdQueue[16][1024];         //for recording the top 16 commands (record)
int lineNum=0;      //the number of cmds
void push(char* cmdline){
    if(lineNum < 16) strcpy(cmdQueue[lineNum++], cmdline);
    else {
        for(int i=0;i<15;i++){
            strcpy(cmdQueue[i], cmdQueue[i+1]);
        }
     strcpy(cmdQueue[15], cmdline);
    }
}



//  Function Declarations for builtin shell commands:
int lsh_cd(char **args);
int lsh_help(char **args);
int lsh_exit(char **args);
int lsh_echo(char **args);
int lsh_record(char **args);
int lsh_replay(char **args);
int lsh_mypid(char **args);

// List of builtin commands, followed by their corresponding functions.
char *builtin_str[] = {
        "cd","help","echo","exit",
        "record","replay","mypid"
};
int (*builtin_func[]) (char **) = {
        &lsh_cd,&lsh_help,&lsh_echo,&lsh_exit,
        &lsh_record,&lsh_replay,&lsh_mypid
};
int lsh_num_builtins() {
    return sizeof(builtin_str) / sizeof(char *);
}

//Builtin function implementations.

//cd:change directory
//args[0]:cd, args[1]:directory(if NULL, return error message)
//call chdir()
int lsh_cd(char **args)
{
    if (args[1] == NULL) {
        fprintf(stderr, "lsh: expected argument to \"cd\"\n");
    }
    else {
        if (chdir(args[1]) != 0) {
            perror("lsh");}
    }
    return 1;
}
//help: print help
int lsh_help(char **args)
{
    printf("------------------------------------------------------------\n");
    printf("My Little Shell\n");
    printf("Type program names and arguments, and hit enter.\n\n");

    printf("The following are built in:\n");
    printf("%-12s","1: help:");printf("show all the build-in function info\n");
    printf("%-12s","2: cd:");printf("change directory \n");
    printf("%-12s","3: echo:");printf("echo the strings to standard output\n");
    printf("%-12s","4: record:");printf("show last-16 cmds you typed in\n");
    printf("%-12s","5: replay:");printf("re-execute the cmd showed in record\n");
    printf("%-12s","6: mypid:");printf("find and print process-ids\n");
    printf("%-12s","7: exit:");printf("exit shell\n\n");

    printf("Use the man command for information on other programs.\n");
    printf("------------------------------------------------------------\n");
    return 1;
}

//exit, return 0 to terminate the execution
int lsh_exit(char **args)
{
    printf("my little shell: see you next time!");
    status=0;
    return 0;
}

//echo
int lsh_echo(char **args){
    int start=1;
    if(cmdnum==1)return 1;

    if(strcmp(args[1],"-n")==0)
        start=2;

    for(;start < cmdnum;start++){
       if(start!=cmdnum-1)
           printf("%s ",args[start]);
       else if(strcmp(args[1],"-n")!=0)
           printf("%s\n",args[start]);
       else
           printf("%s",args[start]);
    }
    return 1;}

//record
int lsh_record(char **args){
     printf("history cmd:\n");
    for (int i=0;i<lineNum;i++){
        printf("%2d:%s\n",i+1,cmdQueue[i]);
    }
    return 1;};

//replay
char *str_replace (char *source, char *find,  char *rep){
   int find_L=strlen(find);
   int rep_L=strlen(rep);
   int length=strlen(source)+1;

   int gap=0;

   char *result = (char*)malloc(sizeof(char) * length);
   strcpy(result, source);

   char *former=source;
   char *location= strstr(former, find);

   while(location!=NULL){
       gap+=(location - former);
       result[gap]='\0';
       length+=(rep_L-find_L);
       result = (char*)realloc(result, length * sizeof(char));
       strcat(result, rep);
       gap+=rep_L;
       former=location+find_L;
       strcat(result, former);
       location= strstr(former, find);
   }

   return result;
}
int lsh_replay(char **args){
    char originStr[1024];
    int num=atoi(args[1]);
    if(num>16||num<1)
        {
        printf("replay: wrong args");
        strcpy(cmdQueue[--lineNum],NULL);
        return 1;
        }

        strcpy(originStr,cmdQueue[num-1]);
    char *str= (char*)malloc(sizeof(char) * strlen(cmdQueue[lineNum-1]));
    strcat(str,args[0]);
    strcat(str," ");
    strcat(str,args[1]);
    char *result=str_replace(cmdQueue[lineNum-1],str,cmdQueue[num-1]);
    strcpy(cmdQueue[lineNum-1],result);
    run(cmdQueue[num-1]);

    strcpy(cmdQueue[num-1],originStr);
    return 1;}


int lsh_mypid(char **args){
    FILE *fp;
    DIR * dir;
    char path [100];
    struct dirent *ptr;

    
    if(strcmp(args[1],"-i")==0)
        printf("%d\n",getpid());
    else if(strcmp(args[1],"-p")==0){
        char temp[15];
        int i=7;
        dir= opendir("/proc/");
        int nofound=1;

        while((ptr=readdir(dir))!=NULL){
            if(strcmp(args[2],ptr->d_name)==0){
                nofound=0;
                closedir(dir);

                strcpy(path,"/proc/");
                strcat(path,args[2]);
                strcat(path,"/status");
                
                fp = fopen(path, "r");
                if(fp!=NULL){
                    while(i>0){
                        strcpy(path,"");
                        fgets(path,100,fp);
                        i--;
                    }
                    while(path[i+6]!='\0'){
                        temp[i] = path[i+6];
                        i++;
                    }
                    temp[i]='\0';
                    printf("%s",temp);
                }
                fclose(fp);
            }
        }
        if(nofound){
            printf("mypid -p: process id not exist.\n");
        } 
    }
    else if(strcmp(args[1],"-c")==0){
            int nofound=1;
            char* pid=args[2];
            
            dir = opendir("/proc/");
            while((ptr=readdir(dir))!=NULL){
                if(strcmp(args[2],ptr->d_name)==0){
                    nofound=0;
                    closedir(dir);

                    strcpy(path,"/proc/");
                    strcat(path,pid);
                    strcat(path,"/task/");
                    strcat(path,pid);
                    strcat(path,"/children");
            
                    fp = fopen(path, "r");
                    if(fp!=NULL){
                        while (fgets(path,100,fp))
                        {
                            printf("%s",path);
                            strcpy(path,"");
                        }
                        printf("\n");
                    }
                    fclose(fp);
                }
            }
            if(nofound)printf("mypid -c: process id not exist\n");
    }
    return 1;}

//for pipe
void cleanup(int n)
{
	int i;
	for (i = 0; i < n; ++i)
		wait(NULL);
}

//launch
int lsh_launch(char **args,int input,int first, int last)
{
    int status;
    pid_t pid,wpid;
    int multipipe[2];
    pipe(multipipe);
    pid = fork();
    if (pid == 0) {
 
        // Child process
        if ((first == 1 && last == 0 && input == 0 )||(first == 0 && last == 0 && input == 0)) {
            // First command with no Input || Commands with no input
			dup2( multipipe[1], STDOUT_FILENO );
		}
        else if(first == 1 && last == 0 && input == 1){
            // First command with Input
            int Ifile = open(args[inPos+1],O_RDONLY);
                if(Ifile==-1){printf("Open failed!\n");return 1;}
            dup2(Ifile, STDIN_FILENO);
            dup2(multipipe[1],STDOUT_FILENO);

            strcpy(args[inPos], args[inPos+1]);
            args[inPos+1]=NULL;
            cmdnum--;
        }
        else if (first == 0 && last == 0 && input != 0) {
			// Middle command with Input
			dup2(input, STDIN_FILENO);
			dup2(multipipe[1], STDOUT_FILENO);
		}
        else if(outPos==0 && inPos==0 && first==1 && last==1)
                {close(multipipe[0]);close(multipipe[1]);}
        else {
            //Last command (include only One commad)
            if(inPos!=0){       //Only One command (with Input)
                int Ifile = open(args[inPos+1],O_RDONLY);
                 if(Ifile==-1){printf("One command Open failed!\n");return 1;}
                dup2(Ifile, STDIN_FILENO);
                strcpy(args[inPos], args[inPos+1]);
                args[inPos+1]=NULL;

                close(Ifile);
            }
            else if(input!=0)
			    dup2(input, STDIN_FILENO );

            if(outPos!=0){      //command with Output
                int Ofile = open(args[outPos+1],O_CREAT | O_WRONLY,0777);
                    if(Ofile==-1){printf("Write failed!\n");return 1;}
                dup2(Ofile, STDOUT_FILENO);
                args[outPos]=NULL;
                args[outPos+1] = NULL;

                close(Ofile);
            }      
        }
    
        if (execvp(args[0], args) == -1) {
            perror("lsh");
        }
        exit(EXIT_FAILURE);
    } else if (pid < 0) {
        // Error forking
        perror("lsh");
    } else {
        // Parent process
        if(!outPos && !inPos && first && last)
        {close(multipipe[0]);close(multipipe[1]);}
        else{
            if (input != 0 && inPos==0){    //close multipipe[0] (can't be the first command)
                        close(input);
                    }
            if(inPos!=0){       //after first command, reset inPos
                inPos=0;
            }
            // Nothing more needs to be written
            close(multipipe[1]);
            // // If it's the last command, nothing more needs to be read
            if (last == 1)
                {close(multipipe[0]);outPos=0;inPos=0;}     //reset inPos and outPos
        }
                   
        do {
                 wpid=waitpid(pid, &status, WUNTRACED);
            } while (!WIFEXITED(status) && !WIFSIGNALED(status));       //WIFEXITED:returns a nonzero value if the child process terminated normally with `exit' or `_exit'
                                                // WIFSIGNALED:returns a nonzero value if the child process terminated because it received a signal that was not handled
             
    }

    return multipipe[0];
}

int N=0;  //number of calls to 'command'
int piping=0;   //if there's multipipe

//Execute shell built-in or launch program.
int lsh_execute(char *cmd,int input, int first, int last)
{
    int i;

    char **args=lsh_split_line(cmd);;
   
    if (args[0] == NULL) {
        // An empty command was entered.
        return 1;
    }

    for (i = 0; i < lsh_num_builtins(); i++) {          //built in command
        if (strcmp(args[0], builtin_str[i]) == 0) {
            int a= (*builtin_func[i])(args);
            if(a && piping) {    N++;return 0;}         //increasing the number of cmd in the multipipe, and make the input be 0
            return a;           //else return status
        }
    }

    for(int i=0;i<cmdnum;i++){              //check ">" and "<"
            if(strcmp(args[i],"<")==0){
                inPos=i;
                input=1;
            }
            if(strcmp(args[i],">")==0){
                outPos=i;
            }
    }
       
    N++;
    return lsh_launch(args,input,first, last);
}


#define LSH_RL_BUFSIZE 1024
/*
   @brief Read a line of input from stdin.
   @return The line from stdin.
 */
char *lsh_read_line(void)
{
    int bufsize = LSH_RL_BUFSIZE;
    int position = 0;
    char *buffer = malloc(sizeof(char) * bufsize);
    int c;

    if (!buffer) {
        fprintf(stderr, "lsh: allocation error\n");
        exit(EXIT_FAILURE);
    }

    while (1) {
        // Read a character
        c = getchar();

        // If we hit EOF, replace it with a null character and return.
        if (c == EOF || c == '\n') {
            buffer[position] = '\0';
            return buffer;
        } else {
            buffer[position] = c;
        }
        position++;

        // If we have exceeded the buffer, reallocate.
        if (position >= bufsize) {
            bufsize += LSH_RL_BUFSIZE;
            buffer = realloc(buffer, bufsize);
            if (!buffer) {
                fprintf(stderr, "lsh: allocation error\n");
                exit(EXIT_FAILURE);
            }
        }
    }
}

#define LSH_TOK_BUFSIZE 64
#define LSH_TOK_DELIM " \t\r\n\a"
/*
   @brief Split a line into tokens (very naively).
   @param line The line.
   @return Null-terminated array of tokens.
 */

char **lsh_split_line(char *line)
{
    int bufsize = LSH_TOK_BUFSIZE;//position = 0;
    char **tokens = malloc(bufsize * sizeof(char*));
    char *token;
    cmdnum=0;
    if (!tokens) {
        fprintf(stderr, "lsh: allocation error\n");
        exit(EXIT_FAILURE);
    }

    token = strtok(line, LSH_TOK_DELIM);
    while (token != NULL) {
        tokens[cmdnum] = token;
        cmdnum++;

        if (cmdnum >= bufsize) {
            bufsize += LSH_TOK_BUFSIZE;
            tokens = realloc(tokens, bufsize * sizeof(char*));
            if (!tokens) {
                fprintf(stderr, "lsh: allocation error\n");
                exit(EXIT_FAILURE);
            }
        }

        token = strtok(NULL, LSH_TOK_DELIM);
    }
    tokens[cmdnum] = NULL;
    return tokens;
}


//the cmd run in background
int builtinBack=0;
void cmdinbackground(char* line){
pid_t pid,wpid;
int status;
int forpid[2];
pipe(forpid);

pid = fork();
line[strlen(line)-1]=NULL;

if (pid ==0){
    close(forpid[0]);  //no use of read
    int x=getpid();
    if(write(forpid[1],&x,sizeof(int))==-1)return 1;
    close(forpid[1]);
    run(line);
    exit(1);
    }


else if (pid < 0) {
    // Error forking
    perror("lsh");}
else{
    close(forpid[1]);
    int y;
    if(read(forpid[0],&y,sizeof(int))==-1)return 1;
    printf("[pid]: %d\n",y);            //parent process return the pid of child process
    close(forpid[0]);

return 0;
}}

int run(char* line){
    piping=0;
    int input=0;
    int first=1;
    char*cmd = line;
    char*next =strchr(cmd,'|');

    while(next != NULL){
        piping=1;
        *next = '\0';
        input = lsh_execute(cmd,input,first,0);
        cmd = next + 1;
        next = strchr(cmd,'|');
        first= 0;
    }
    input=lsh_execute(cmd, input, first, 1);
    cleanup(N);
    N=0;
   
    return input;
}

/*
   @brief Loop getting input and executing it.
 */
void lsh_loop(void)
{
    char *line;
    status=1;


    do {
        printf(">>> $ ");
        line = lsh_read_line();
            push(line);
            if(strpbrk(line, "&")!=NULL) cmdinbackground(line);
            else run(line);
            
        
    
    } while (status);
}

/**
   @brief Main entry point.
   @param argc Argument count.
   @param argv Argument vector.
   @return status code
 */
int main(int argc, char **argv)
{
    // Run command loop.
    lsh_loop();
    return EXIT_SUCCESS;
}

