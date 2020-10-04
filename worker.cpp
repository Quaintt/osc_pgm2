/*
    file: worker.cpp
    author: Gherkin
    modification history: 
        Gherkin
        October 3rd, 2020
    procedures:
        main - Initialize nessisary components and calls spawnProcesses.
        cleanup - Waits for all child processes to complete then frees resources.
        producer - Generates single char messages and places them into the buffer.
        consumer - Reads message from the buffer, removing the message after reading.
        spawnProcesses - Forks a given number of children from the parent process.
*/
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

#define ITEMS 1000000

//Color codes for printing in terminal
#define RED "\x1B[31m"
#define GRN "\x1B[32m"
#define RESET "\x1B[0m"

int pid[12];
sem_t sem1, sem2;
struct mem *sm;

void spawnProcesses(int processes); //Forward declaration so the compiler doesn't scream at me.

struct mem
{
    sem_t bufferReady;
    sem_t mSpaceReady;
    sem_t accessReady;
    sem_t totalActionsPerformed;
    sem_t itemsAvailable;
    int bItems;
    char buffer[1000];
};

/*
    int main()
    author: Gherkin
    date: Oct 3, 2020
    description: Generates the file descriptor and shared memory, then initializes nessisary variables.
        Finally, it calls spawnProcesses().
*/
int main()
{
    // init data
    int fileDescriptor = shm_open("ledger", O_CREAT | O_RDWR, S_IRUSR | S_IWUSR);
    off_t structSize = sizeof(mem);
    ftruncate(fileDescriptor, structSize);
    mem *sharedMemory = (mem *)mmap(NULL, structSize, PROT_READ | PROT_WRITE, MAP_SHARED, fileDescriptor, 0);
    if (sharedMemory == MAP_FAILED)
        printf("mmap error code: %d \n", errno);

    //init semaphores
    sem_init(&(sharedMemory->bufferReady), 1, 1);
    sem_init(&(sharedMemory->mSpaceReady), 1, 1000);
    sem_init(&(sharedMemory->itemsAvailable), 1, 0);
    sem_init(&(sharedMemory->totalActionsPerformed), 1, 0);
    sem_init(&(sharedMemory->accessReady), 1, 1);
    sharedMemory->bItems = 0;

    sm = sharedMemory;

    pid[12] = getpid();
    spawnProcesses(11);

    return 0;
}

/*
    void cleanup()
    author: Gherkin
    date: Oct 3, 2020
    description: Has the parent process wait for all the child processes to finish, then clears the
        nessisary variables.
*/
void cleanup()
{

    for (int j = 0; j < 12; j++) //loop through each child process
    {
        waitpid(pid[j], NULL, 0); //reaper function
    }

    sem_close(&(sm->bufferReady));
    sem_close(&(sm->mSpaceReady));
    sem_close(&(sm->itemsAvailable));
    sem_close(&(sm->accessReady));
    sem_close(&(sm->totalActionsPerformed));

    munmap(NULL, sizeof(mem));
    shm_unlink("ledger");

    printf(GRN "~~ cleanup complete ~~\n" RESET);
    exit(EXIT_SUCCESS);
}

/*
    void consumer(int id)
    author: Gherkin
    date: Oct 3, 2020
    description: Reads messages that were placed into the buffer by the producer. After reading, it
        replaces the message with the consumer's id to indicate that the message has been cleared.
    parameters:
        id         I/P   int   The number identifier of the current consumer process
        consumer   O/P   int   Total number of messages recieved from producer
*/
int consumer(int id)
{
    int actionsPerformed = 0;
    int itemsRecieved = 0;
    int item;
    while (actionsPerformed < ITEMS / 10)
    {
        //wait until it's okay to access data
        sem_wait(&(sm->bufferReady));

        //how many items available?
        sem_wait(&(sm->itemsAvailable));
        itemsRecieved++; //add to the items recieved count

        //wait for its turn to read/write
        sem_wait(&(sm->accessReady));

        //read and modify/clear data
        item = sm->bItems;
        char k = sm->buffer[item - 1];
        sm->buffer[item - 1] = id; //replace producer's message with consumer id to indicate message has been taken.
        sm->bItems -= 1;
        sem_post(&(sm->accessReady));
        if (k != 63) //if message was not recived
        {
            printf(RED "!!! consumer %d ~ buffer[%d]: " RESET, id, (item - 1));
            printf(RED "Existing Value %d\n" RESET, k);
            itemsRecieved--; //nullify the most recent item addition
        }

        //update the actions counters
        actionsPerformed += 1;

        //tell producer that data has been changed
        sem_post(&(sm->mSpaceReady));
    }
    return itemsRecieved;
}

/*
    void producer()
    author: Gherkin
    date: Oct 3, 2020
    description: Creates a set number of messages and places them into the buffer.
*/
void producer()
{
    int itemsProduced = 0;
    int offset = 0;
    char message[1000];

    //Create char array the size of the buffer filled with single char messages.
    //Before writing to the buffer, this array will be truncated based on an offset value.
    for (int x = 0; x < 1000; x++)
    {
        message[x] = '?'; //the "message" is just a question mark
    }

    while (itemsProduced < ITEMS)
    {
        //wait for space in memory to be cleared up?
        sem_wait(&(sm->mSpaceReady));

        //retrive offset value
        sem_getvalue(&(sm->mSpaceReady), &offset);
        size_t s = offset;

        //placing data into shared memory buffer
        sem_wait(&(sm->accessReady)); //only one thing can write to mem at a time
        memcpy(&sm->buffer, message, strlen(message) - offset); //truncate message size based on offset
        itemsProduced++;
        sm->bItems += 1;
        sem_post(&(sm->accessReady));

        //increase the amount of items available
        sem_post(&(sm->itemsAvailable));

        //tell consumer that it is safe to access memory
        sem_post(&(sm->bufferReady));
    }
}

/*
    void spawnProcesses(int processes)
    author: Gherkin
    date: Oct 3, 2020
    description: Uses fork to create producer and consumer child processes, then sends the parent
        process to cleanup().
    parameters:
        processes   I/P   int   The number of processes that will be created
*/
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
            printf(RESET "Producer " GRN "%d" RESET " ended.\n", c);
            break;
        }
        else if (pid[c] == 0)
        { //child
            int output = consumer(c);
            double acu = (double)output / ITEMS * 1000;
            printf(RESET "Consumer " GRN "%d" RESET " ended. Items obtained: " GRN "%d" RESET, c, output);
            printf(" Accuracy: " GRN "%f%%\n" RESET, acu);
            break;
        }
    }

    if (c != (processes + 1))
        exit(EXIT_SUCCESS); //if not the parent

    //only the parent process will make it this far
    cleanup();
}
