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

class worker
{
private:
    /* data */
public:
    worker(/* args */);
    ~worker();
};

worker::worker(/* args */)
{
}

worker::~worker()
{
}

struct consumer
{
    /* data */
};

struct producer
{
    /* data */
};

struct data
{
    /* data */
};

