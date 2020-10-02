#include <fcntl.h>    /* For O_* constants */
#include <sys/stat.h> /* For mode constants */
#include <semaphore.h>
#include <stdio.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>
#include <string.h>
#include <signal.h>
#include <errno.h>
#include <sys/shm.h>
#include <sys/mman.h>

//colors because ofc.
#define RED "\x1B[31m"
#define GRN "\x1B[32m"
#define YEL "\x1B[33m"
#define BLU "\x1B[34m"
#define MAG "\x1B[35m"
#define CYN "\x1B[36m"
#define WHT "\x1B[37m"
#define RESET "\x1B[0m"

/*
1 producer, 10 consumers. All run using fork commands.
Buffer size 1,000.
Producer executes 1,000,000 times.
Normal exit after all items are consumed. Everything needs to be cleaned up before terminating.
Must use linux semaphores.
*/

/*
Using the VM for Ubuntu 20.04 LTS from Pgm1, write a C/C++ program which simulates the Producer/Consumer 
program in Figure 5.16 with a buffer size of one thousand. Allow the Producer to generate one million 
items. Use ten consumers. The program needs to perform a normal exit process after all items are 
consumed. Both the Producer (singular) and Consumers are to be runs as separate processes generated via 
fork(). The program must us Linux semaphores. The program must clean up the semaphores used and zombies 
created before termination. Report the number of items that each consumer obtained. Do the counts meet 
expectations?
*/

const int sizeOfBuffer = 1;
const int totalItems = 50;
//sem_t s = 1, n = 0, e = sizeofbuffer;
sem_t s, n, e;

void produ()
{
    pid_t pid = getpid();
    for (int x = 0; x < totalItems; x++)
    {
        printf(BLU "[produ]\t" RESET "waiting...\n");
        sem_wait(&e);
        sem_wait(&s);
        sleep(0.1);
        //append();

        int value1, value2, value3;
        sem_getvalue(&s, &value1);
        sem_getvalue(&n, &value2);
        sem_getvalue(&e, &value3);
        printf(BLU "[produ]\t" CYN "Sending... One: %d N: %d e: %d" RESET "\n", value1, value2, value3);

        sem_post(&s);
        sem_post(&n);
        //printf(BLU "[produ]\t" RESET "sent.\n");
    }

    return;
    /*
    for (int p = 8; p >= 0; p--){
        kill(pid + p, SIGKILL);
    }
    */
    //kill(pid, SIGKILL);
}

int consu()
{
    pid_t pid = getpid();
    int obtained = 1;
    bool isOneMillion = false;
    while (!isOneMillion)
    {
        if (sem_wait(&e) != 0 || sem_wait(&s) != 0)
        {
            printf(GRN "[consu %d]\t" RESET "waiting...\n", pid);
        }
        //sleep(0.05);
        sem_post(&s);
        sem_post(&n);

        int value1, value2, value3;
        sem_getvalue(&s, &value1);
        sem_getvalue(&n, &value2);
        sem_getvalue(&e, &value3);
        //printf(GRN "[consu %d]\t" RESET "...recieved", pid);
        //printf(CYN "\tOne: %d N: %d e: %d\n" RESET, value1, value2, value3);
        
        obtained++;
        if (value2 >= totalItems)
            isOneMillion = true;
    }
    return obtained;
}

void spawnConsumers(int c)
{ //creates c + 2 consumers???
    /*
    if (fork() == 0)
    {
        //child
        pid_t pid = getpid();
        printf(RED "Created new Consumer! ~ PID: %d\n" RESET, pid);
        int output = consu();
        printf("Consumer " RED "%d" RESET " ended. Total items obtained: " RED "%d\n" RESET, pid, output);
    }
    else{
        //parent
        printf("C = %d\n",c);
        if (c > 0) {
            c--;
            spawnConsumers(c);
        }
    }
    */

    //TODO: Change stolen code
    pid_t consu_pid;
    for (c; c > 0; c--)
    {
        //pid_t pid = fork();
        consu_pid = fork();
        if (consu_pid == 0)
        { //child
            pid_t temp_pid = getpid();
            printf(RED "Created Consumer %d! ~ PID: %d\n" RESET, c, temp_pid);
            int output = consu();
            printf("Consumer " RED "%d" RESET " ended. Total items obtained: " RED "%d\n" RESET, temp_pid, output);
            break;
        }
        else if (consu_pid < 0)
        { //error?
            //unlink semaphore
            //close semaphore
        }
    }

    while (consu_pid = waitpid(-1, NULL, 0))
    {
        if (errno == ECHILD)
        {
            break;
        }
    }
    printf(GRN "~~~ All children have ended. ~~~\n" RESET);
}

int main()
{
    sem_init(&s, 0, 1);            //enforces mutual exclusion
    sem_init(&n, 0, sizeOfBuffer); //size of buffer
    sem_init(&e, 0, totalItems);   //tracks empty spaces

    pid_t gigafork = fork();
    if (gigafork == 0)
    {
        //giga-child that handles all of the different elements
        pid_t pid = getpid();
        printf(GRN "Created original Consumer! ~ PID: %d\n" RESET, pid);
        spawnConsumers(10);
        printf(GRN "Parent finished loop! ~ PID: %d\n" RESET, pid);
        //kill(pid, SIGKILL);
        kill(getpid(), SIGTERM);
        return 2;
        
        /*
        if (fork() == 0)
        {
            //child
            pid_t pid = getpid();
            printf(GRN "Created original Consumer! ~ PID: %d\n" RESET, pid);
            spawnConsumers(10);
            printf(GRN "Parent finished loop! ~ PID: %d\n" RESET, pid);
            //kill(pid, SIGKILL);
            kill(getpid(), SIGTERM);
            return 2;
        }
        else
        {
            //parent
            pid_t pid = getpid();
            printf("Created Producer! ~ PID: %d\n", pid);
            sleep(0.2);
            produ();
            sem_destroy(&s);
            sem_destroy(&n);
            sem_destroy(&e);
            kill(getpid(), SIGTERM);
            return 0;
        }
        */
    }
    else if (gigafork != 0)
    {
        //giga-parent
        pid_t pid = getpid();
        printf("Created Producer! ~ PID: %d\n", pid);
        sleep(0.2);
        produ();
        
        

        //TODO: change stolen code
        while (gigafork = waitpid(-1, NULL, 0))
        {
            if (errno == ECHILD)
            {
                break;
            }
        }
        printf(GRN "~~~ All children have ended. ~~~\n" RESET);

        // ADD CODE TO CLEAN UP EVERYTHING

        sem_destroy(&s);
        sem_destroy(&n);
        sem_destroy(&e);
        kill(getpid(), SIGTERM);
        
    }
    //kill(getpid(), SIGTERM);
    return 1;
}

/*
const int sizeOfBuffer = 1000;
semaphore s = 1, n = 0, e = sizeofbuffer;

void producer(){
    while (true){
        produce();
        semWait(e);
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
        semSignal(e);
        consume();
    }
}

void main(){
    parbegin(producer, consumer);
}
*/
