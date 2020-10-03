#include <fcntl.h>
#include <sys/stat.h>
#include <semaphore.h>
#include <stdio.h>
#include <sys/types.h>
#include <unistd.h>
#include <string.h>
#include <signal.h>
#include <errno.h>
#include <sys/wait.h>
#include <sys/shm.h>
#include <sys/mman.h>
#include <stdlib.h>

//colors because ofc.
#define RED "\x1B[31m"
#define GRN "\x1B[32m"
#define YEL "\x1B[33m"
#define BLU "\x1B[34m"
#define MAG "\x1B[35m"
#define CYN "\x1B[36m"
#define WHT "\x1B[37m"
#define RESET "\x1B[0m"

int pid[12];
const char ledger[] = "ledger";
sem_t sem1, sem2;
struct mem* hell;
//pid_t pid;

void spawnProcesses(int processes); //Forward declaration so the compiler doesn't scream at me.

struct mem
{
    int actionsPerformed;
    sem_t s1;
    sem_t s2;
    size_t bytes_in_buffer;
    char buffer[1000]; //I'm just assuming he ment 1000 bytes and not 1000 items...
};

int main()
{
    // init data
    // init semaphores
      
    int fileDescriptor = shm_open("ledger", O_CREAT | O_RDWR, S_IRUSR | S_IWUSR); //not sure what S_IRUSR or S_IWUSR do
    off_t structSize = sizeof(mem);
    //off_t structSize = 2*sysconf(_SC_PAGE_SIZE);
    ftruncate(fileDescriptor, structSize);
    mem* gamer = (mem*)mmap(NULL, structSize, PROT_READ | PROT_WRITE, MAP_SHARED, fileDescriptor, 0);
    if(gamer == MAP_FAILED) printf("mmap error code: %d \n",errno);
    sem_init(&(gamer->s1), 1, 0);
    sem_init(&(gamer->s2), 1, 0);

    //addr = (char*)mmap(NULL, structSize, PROT_READ | PROT_WRITE, MAP_SHARED, fileDescriptor, 0);
    //if(addr == MAP_FAILED) printf("fuck %d \n",errno);

    hell = gamer;

    pid[12] = getpid();
    spawnProcesses(11);

    return 0;
}

void cleanup()
{

    for (int j = 0; j < 12; j++)
    {
        waitpid(pid[j], NULL, 0); //reaper function
    }

    while (waitpid(-1, NULL, 0) == 0) //I changed this so it probably doesn't work now
    {
        if (errno == ECHILD)
        {
            break;
        }
    }

    //shm_unlink();

    printf(GRN "~~ cleanup complete ~~\n" RESET);
    exit(EXIT_SUCCESS);
}

int consumer(char test)
{
    while (true)
    {
        //wait untill it's okay to access data
        sem_wait(&(hell->s1));

        //sleep(1);

        //read the data?
        for(int z = 0; z < hell->bytes_in_buffer; z++){
            hell->buffer[z] = test;
        }
        //printf("changed data :)\n");

        //tell producer that data has been changed
        sem_post(&(hell->s2));

    }
}

void producer()
{
    int fileDescriptor = shm_open("ledger", O_CREAT | O_RDWR, S_IRUSR | S_IWUSR); 
    off_t structSize = sizeof(mem);
    ftruncate(fileDescriptor, structSize);
    mem* prodMemLink = (mem*)mmap(NULL, structSize, PROT_READ | PROT_WRITE, MAP_SHARED, fileDescriptor, 0);
    if(prodMemLink == MAP_FAILED) printf("mmap error code: %d \n",errno);
    char message[] = "?";
    
    while (true)
    {
        //placing data into shared memory buffer
        prodMemLink->bytes_in_buffer = strlen(message);
        memcpy(&prodMemLink->buffer, message, strlen(message));

        //tell consumer that it is safe to access memory
        sem_post(&(prodMemLink->s1));

        //wait for space in memory to be cleared up?
        sem_wait(&(prodMemLink->s2));

        printf("char at [0]: %d\n", prodMemLink->buffer[0]);

        sleep(1);

        //make item
        //check if space in memory
        //write to memory
        //indicate that new stuff is in memory


    }
}

void spawnProcesses(int processes) // one process will be the producer, so 11 total is needed
{
    int c = 1;
    for (c; c <= processes; c++)
    {
        pid[c] = 0;
        pid[c] = fork();
        if (pid[c] == 0 && c == processes) //child and first loop
        {
            producer();
            printf(RESET "Producer " RED "%d" RESET " ended.\n", c);
            break;
        }
        else if (pid[c] == 0)
        { //child
            //consider storing the number of the child in the struct??
            char t = '0' + c;
            int output = consumer(t);
            printf(RESET "Consumer " RED "%d" RESET " ended. Total items obtained: " RED "%d\n" RESET, c, output);
            break;
        }
    }

    if (c != (processes + 1))
        exit(EXIT_SUCCESS); //if not the parent

    //only the parent process will make it this far
    cleanup();
}
