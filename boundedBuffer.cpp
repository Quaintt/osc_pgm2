#include <fcntl.h>    /* For O_* constants */
#include <sys/stat.h> /* For mode constants */
#include <semaphore.h>
#include <stdio.h>
#include <sys/types.h>
#include <unistd.h>
#include <string.h>
#include <signal.h>

//colors because ofc.
#define RED   "\x1B[31m"
#define GRN   "\x1B[32m"
#define YEL   "\x1B[33m"
#define BLU   "\x1B[34m"
#define MAG   "\x1B[35m"
#define CYN   "\x1B[36m"
#define WHT   "\x1B[37m"
#define RESET "\x1B[0m"

/*
1 producer, 10 consumers. All run using fork commands.
Buffer size 1,000.
Producer executes 1,000,000 times.
Normal exit after all items are consumed. Everything needs to be cleaned up before terminating.
Must use linux semaphores.
*/
const int sizeOfBuffer = 1;
const int totalItems = 50;
//sem_t s = 1, n = 0, cmdNum = sizeofbuffer;
sem_t one, n, cmdNum;

void produ()
{
    pid_t pid = getpid();
    for (int x = 0; x < totalItems; x++)
    {
        //what does produce() do?
        printf(BLU "[produ]\t" RESET "waiting...\n");
        sem_wait(&cmdNum);
        sem_wait(&one);
        //sleep(1);
        //append();

        int value1, value2, value3; 
        sem_getvalue(&one, &value1);
        sem_getvalue(&n, &value2);
        sem_getvalue(&cmdNum, &value3);
        printf(BLU "[produ]\t" CYN "One: %d N: %d cmdNum: %d\n" RESET, value1, value2, value3);

        sem_post(&one);
        sem_post(&n);
        //printf(BLU "[produ]\t" RESET "sent.\n");
    }
    /*
    for (int p = 8; p >= 0; p--){
        kill(pid + p, SIGKILL);
    }
    */
   //kill(pid, SIGKILL);
}

void consu()
{
    pid_t pid = getpid();
    bool isOneMillion = false;
    while (!isOneMillion)
    {
        //printf(GRN "[consu %d]\t" RESET "waiting...\n", pid);
        //sem_wait(&cmdNum);
        //sem_wait(&one);
        if (sem_wait(&cmdNum) != 0 || sem_wait(&one) != 0){
            printf(GRN "[consu %d]\t" RESET "waiting...\n", pid);
        }
        //sleep(1);
        sem_post(&one);
        sem_post(&n);
        printf(GRN "[consu %d]\t" RESET "...recieved\n", pid);
        int value2; 
        sem_getvalue(&n, &value2);
        if(value2 >= totalItems) isOneMillion = true;
        //if(!isOneMillion){isOneMillion = true;}
    }
}

void spawnConsumers(int c){ //creates c + 2 consumers???
    if (fork() == 0)
    {
        //child
        pid_t pid = getpid();
        printf(RED "Created new Consumer! ~ PID: %d\n" RESET, pid);
        consu();
    }
    else{
        //parent
        printf("C = %d\n",c);
        if (c > 0) {
            c--;
            spawnConsumers(c);
        }
    }
}

int main()
{
    sem_init(&one, 0, sizeOfBuffer);
    sem_init(&n, 0, 1);
    sem_init(&cmdNum, 0, totalItems);
    if (fork() == 0)
    {
        //child
        pid_t pid = getpid();
        printf(RED "Created original Consumer! ~ PID: %d\n" RESET, pid);
        spawnConsumers(8);
        printf(RED "Parent finished loop! ~ PID: %d\n" RESET, pid);
        //kill(pid, SIGKILL);
    }
    else
    {
        //parent
        pid_t pid = getpid();
        printf("Created Producer! ~ PID: %d\n", pid);
        sleep(1);
        produ();
        sem_destroy(&one);
        sem_destroy(&n);
        sem_destroy(&cmdNum);
    }
    printf("yo.\n");
    kill(getpid(), SIGTERM);
}

/*
const int sizeOfBuffer = 1000;
semaphore s = 1, n = 0, cmdNum = sizeofbuffer;

void producer(){
    while (true){
        produce();
        semWait(cmdNum);
        semWait(s);
        append();
        semSignal(s);
        semSignal(n);
    }
}

void consumer(){
    while (true){
        semWait(n);
        semWait(s);
        take();
        semSignal(s);
        semSignal(cmdNum);
        consume();
    }
}

void main(){
    parbegin(producer, consumer);
}
*/
