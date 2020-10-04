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
struct mem *hell; //TODO: Rename dumb var names
const int totalItemsProduced = ITEMS;
//pid_t pid;

void spawnProcesses(int processes); //Forward declaration so the compiler doesn't scream at me.

struct mem
{
    sem_t bufferReady;
    sem_t mSpaceReady;
    sem_t accessReady;
    sem_t totalActionsPerformed;
    sem_t itemsAvailable;
    size_t bytes_in_buffer;
    int bItems;
    char buffer[ITEMS]; //I'm just assuming he ment 1000 bytes and not 1000 items...
};

int main()
{
    // init data
    int fileDescriptor = shm_open("ledger", O_CREAT | O_RDWR, S_IRUSR | S_IWUSR); //not sure what S_IRUSR or S_IWUSR do
    off_t structSize = sizeof(mem);
    ftruncate(fileDescriptor, structSize);
    mem *gamer = (mem *)mmap(NULL, structSize, PROT_READ | PROT_WRITE, MAP_SHARED, fileDescriptor, 0);
    if (gamer == MAP_FAILED)
        printf("mmap error code: %d \n", errno);

    //init semaphores
    sem_init(&(gamer->bufferReady), 1, 1);
    sem_init(&(gamer->mSpaceReady), 1, 1000);
    sem_init(&(gamer->itemsAvailable), 1, 0);
    sem_init(&(gamer->totalActionsPerformed), 1, 0);
    sem_init(&(gamer->accessReady), 1, 1);
    gamer->bItems = 0;

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

    sem_close(&(hell->bufferReady));
    sem_close(&(hell->mSpaceReady));
    sem_close(&(hell->itemsAvailable));
    sem_close(&(hell->accessReady));
    sem_close(&(hell->totalActionsPerformed));

    munmap(NULL, sizeof(mem));
    shm_unlink("ledger");

    printf(GRN "~~ cleanup complete ~~\n" RESET);
    exit(EXIT_SUCCESS);
}

int consumer(int id)
{
    int actionsPerformed = 0;
    int itemsRecieved = 0;
    int item;
    while (actionsPerformed < totalItemsProduced / 10)
    {
        //wait until there are items to retrieve
        //sem_wait(&(hell->itemsAvailable));

        //wait untill it's okay to access data
        sem_wait(&(hell->bufferReady));

        //sleep(0.666);

        //how many items available?
        sem_wait(&(hell->itemsAvailable));
        //sem_getvalue(&(hell->itemsAvailable), &item); //TODO: Make this wayyyy better
        //item++;                                       //compensate for the change from the sem_wait
        itemsRecieved++;                              //add to the items recieved count
        
        //read and modify(clear) data
        sem_wait(&(hell->accessReady));
        //printf(RESET "c%d:" GRN "[%d]%d" RESET, id, (item-1), hell->buffer[item-1]);
        item = hell->bItems;
        char k = hell->buffer[item - 1];
        hell->buffer[item - 1] = id; //if one item available, go to slot zero
        hell->bItems -= 1;
        sem_post(&(hell->accessReady));
        //printf(RED "[%d]%d" RESET, (item-1), hell->buffer[item-1]);
        if (k != 63)
        {
            printf(RED "!!! consumer %d ~ buffer[%d]: " RESET, id, (item-1));
            printf(RED "Existing Value %d\n" RESET, k);
            itemsRecieved--; //nullify the most recent item addition
        }

        //update the actions counters
        actionsPerformed += 1;

        //tell producer that data has been changed
        sem_post(&(hell->mSpaceReady));

        sleep(0.9); //aims to prevent the same consumer from reciving 2 items in a row
    }
    return itemsRecieved;
}

void producer()
{
    int itemsProduced = 0;
    int offset = 0;
    char message[ITEMS];

    for (int x = 0; x < ITEMS; x++)
    {
        message[x] = '?';
    }

    while (itemsProduced < totalItemsProduced)
    {
        //wait for space in memory to be cleared up?
        sem_wait(&(hell->mSpaceReady));

        sem_getvalue(&(hell->mSpaceReady), &offset);
        //offset = totalItemsProduced - offset;

        size_t s = offset;

        //placing data into shared memory buffer
        sem_wait(&(hell->accessReady)); //only one thing can write to mem at a time
        memcpy(&hell->buffer, message, strlen(message) - offset);
        //memset(&hell->buffer, '?', s);
        itemsProduced++;
        hell->bItems += 1;
        sem_post(&(hell->accessReady));

        //increase the amount of items available
        sem_post(&(hell->itemsAvailable));
        int output_text;
        sem_getvalue(&(hell->itemsAvailable), &output_text);
        // printf(RESET "ItmAvl: %d  ", output_text);

        //tell consumer that it is safe to access memory
        sem_post(&(hell->bufferReady));
        sem_getvalue(&(hell->bufferReady), &output_text);
        // printf("buffRdy: %d  ", output_text);

        sem_getvalue(&(hell->mSpaceReady), &output_text);
        // printf("sRdy: %d  ", output_text);

        // printf("prod: %d  ", itemsProduced);

        // printf("char at [0]: %d\n", hell->buffer[0]);
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
            printf(RESET "Producer " GRN "%d" RESET " ended.\n", c);
            break;
        }
        else if (pid[c] == 0)
        { //child
            //char t = '0' + c;
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
