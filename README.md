## OS Concepts Programming Assignment 2

### Problem Statement
The task was to create a C/C++ program that simulated the results and functionality of the sample program provided in figure 5.16 of the textbook. This would require a singular producer process to generate a total of one million items, which would be shared with ten consumer processes via a buffer with a size of 1000. Each of these processes must be created using fork and must utilize Linux semaphores to communicate. After running, each of the consumers must report the number of items they received, and all processes and semaphores must be properly cleaned up. This assignment tests the programmer’s knowledge of semaphore, fork, and memory sharing concepts.

### Approach
I chose to write the program in C++ out of familiarity with the language – something that I figured would benefit me since I was dealing with unfamiliar concepts. I regrettably coded the whole thing using Visual Studio Code within the Ubuntu 20.04 LTS virtual machine I was running off my laptop. This quickly reviled itself to be quite the challenge due to hardware performance issues and eye strain from extensively staring at the 12-inch screen.

### Solution
For readability, I broke much of the code’s functionality into multiple void functions. I started by using textbook figure 5.16 as a guide, attempting to recreate it as close as possible – this quickly turned out to be problematic once attempting to implement a shared memory solution. As a result, my solution follows a skeleton of the textbook example, but primarily deviates in how it handles the delivery of items. The code returns indications of the process’s status by printing to the console. I used red and green color coding to indicated if the output was good or bad accordingly. Any improperly delivered message will be deducted from a consumer’s total of messages received and will also be reflected in the consumer’s total accuracy percentage.

### Build and Execution
```console
gherkin@Gherkin-VM:~/Documents/pgm2$ ls
worker.cpp
gherkin@Gherkin-VM:~/Documents/pgm2$ g++ worker.cpp -o worker -lpthread -lrt
gherkin@Gherkin-VM:~/Documents/pgm2$ ls
worker worker.cpp
gherkin@Gherkin-VM:~/Documents/pgm2$ ./worker
Consumer 1 ended. Items obtained: 100000 Accuracy: 100.000000%
Consumer 6 ended. Items obtained: 100000 Accuracy: 100.000000%
Consumer 7 ended. Items obtained: 100000 Accuracy: 100.000000%
Consumer 9 ended. Items obtained: 100000 Accuracy: 100.000000%
Consumer 4 ended. Items obtained: 100000 Accuracy: 100.000000%
Consumer 2 ended. Items obtained: 100000 Accuracy: 100.000000%
Consumer 3 ended. Items obtained: 100000 Accuracy: 100.000000%
Consumer 8 ended. Items obtained: 100000 Accuracy: 100.000000%
Consumer 10 ended. Items obtained: 100000 Accuracy: 100.000000%
Producer 11 ended.
Consumer 5 ended. Items obtained: 100000 Accuracy: 100.000000%
~~ cleanup complete ~~
gherkin@Gherkin-VM:~/Documents/pgm2$
```
