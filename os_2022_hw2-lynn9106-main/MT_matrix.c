#include <stdio.h> //fprintf() realloc() free() exit() execvp()
#include <stdlib.h> //malloc() realloc() free() exit() execvp() EXIT_SUCCESS ,EXIT_FAILURE, atoi()
#include <string.h> //strcmp() strtok()
#include <unistd.h>
#include <pthread.h>
#include <sys/types.h>
#include <sys/syscall.h>
#include <fcntl.h>
#include <dirent.h> //FILE, DIR
#include <time.h>
#include <sys/time.h>
#define _GNU_SOURCE

#define LSH_TOK_DELIM " \t\r\n\a"
#define BUFSIZE 100
pthread_mutex_t mutex;
char* fname_matrix1;
char* fname_matrix2;
char* thread_num_char;
int thread_num=0;
int matrix1_size=0,matrix2_size=0,matrixres_size=0;
int row1=0,col1=0,row2=0,col2=0;
long long unsigned matrix1[4096][4096];
long long unsigned matrix2[4096][4096];
long long unsigned matrix_res[4096][4096]={0};


void *multi(void* num);

/*read the information of matrix multiplication8*/
void readline(int argc,char* argv[]){
    fname_matrix1 = malloc(sizeof(char) * BUFSIZE);
    fname_matrix2 = malloc(sizeof(char) * BUFSIZE);
    thread_num_char= malloc(sizeof(char)*10);

    strcpy(thread_num_char , argv[1]);
    thread_num=atoi(thread_num_char);
    strcpy(fname_matrix1 , argv[2]);
    strcpy(fname_matrix2 , argv[3]);
}


int main(int argc,char* argv[]){
        FILE *fp;
        readline(argc,argv);

/*read two matrix into matrix1 and matrix2*/
        fp = fopen(fname_matrix1, "r"); if(!fp){printf("open1 failed\n");return 1;}
        fscanf(fp, "%d %d",&row1,&col1);
        matrix1_size=row1*col1;
        for(int i=0;i<row1;i++){
            for(int j=0;j<col1;j++){
                fscanf(fp,"%llu",&matrix1[i][j]);
            }
        }
        fp = fopen(fname_matrix2,"r"); if(!fp){printf("openfile2 failed!\n"); return 1;}
        fscanf(fp, "%d %d",&row2,&col2);
        matrix2_size=row2*col2;
        for(int i=0;i<row2;++i){
            for(int j=0;j<col2;j++){
                fscanf(fp,"%llu",&matrix2[i][j]);
            }
        }
        matrixres_size=row1*col2;


/*create multithreads to do matrix multiplication*/
        pthread_t *threads;
pthread_mutex_init(&mutex,NULL);        /*mutex for critical section*/
        int* th_arr = malloc(thread_num*sizeof(int));
        threads = (pthread_t*)malloc(thread_num*sizeof(pthread_t));

    struct timeval start, end;      //recording elapsed time
    gettimeofday(&start, NULL);

        for(int i=0;i<thread_num;i++){
        //creating threads
            th_arr[i]=i;void *iptr=&th_arr[i];
            pthread_create(&threads[i], NULL,multi,iptr);
        }
        for(int i=0;i<thread_num;i++){
            pthread_join(threads[i], NULL);
        }

    gettimeofday(&end, NULL);
    long seconds = (end.tv_sec - start.tv_sec);
    long micros = ((seconds * 1000000) + end.tv_usec) - (start.tv_usec);
    printf("The elapsed time is %lf seconds\n", ((double)micros/1000000.0));

        fp=fopen("result.txt","w");         /*output the result to result.txt*/
        fprintf(fp,"%d %d\n",row1,col2);
        for(int i=0;i<row1;++i){
            for(int j=0; j<col2;++j){
                fprintf(fp,"%llu ",matrix_res[i][j]);
            }
            fprintf(fp,"\n");
        }
        fclose(fp);


        char result[1025]="";               /*print thread utime and context switch time read from /proc/thread_info*/
        printf("PID: %d\n",getpid());
        int fd=open("/proc/thread_info",O_RDONLY);if(fd<0){printf("file thread_info not found!"); return 1;}
        read(fd,result,1024);
        char *token;
        token = strtok(result,LSH_TOK_DELIM);
        while(token!=NULL){
                printf("    ThreadID:%s ",token);
            token = strtok(NULL,LSH_TOK_DELIM);
                printf("Time:%s(ms) ",token);
            token = strtok(NULL,LSH_TOK_DELIM);
                printf("context switch times:%s\n",token);
            token = strtok(NULL,LSH_TOK_DELIM);
        }
        close(fd);

pthread_mutex_destroy(&mutex);
    return 0;
}

/*matrix multiplication (element dispatch)*/
void *multi(void* num){
    FILE *proc_info;
    int thread_i=(*(int*)num);          /*the number of thread (not thread id)*/

    for(int k=thread_i;k<matrixres_size;k+=thread_num){
            int Q = k/col2;
            int R = k%col2;
            long long unsigned total=0;
            for(int w=0;w<col1;w++){
                total+=matrix1[Q][w]*matrix2[w][R];
            }
            pthread_mutex_lock(&mutex);
            matrix_res[Q][R]=total;
            pthread_mutex_unlock(&mutex);
    }
        pthread_mutex_lock(&mutex);     /*write the pid & tid in /proc/thread_info*/
            proc_info=fopen("/proc/thread_info","w");if(!proc_info){printf("proc_info can't open\n");return;}
            fprintf(proc_info,"%d %d\n",getpid(),gettid());
            fclose(proc_info);
        pthread_mutex_unlock(&mutex);
        pthread_exit(NULL);
}