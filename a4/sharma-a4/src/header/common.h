#include <iostream>
#include <unistd.h>
#include <chrono>
#include <pthread.h>
#include <stdio.h>
#include <fstream>
#include <vector>
#include <string.h>
#include <iostream>
#include <iterator>
#include <semaphore.h>
#include <unordered_map>


using namespace std;
using namespace std::chrono;

#define NRES_TYPES 10
#define NTASKS 25
#define STR_SIZE 32
#define MAXLINE 128

enum state{WAIT, RUN, IDLE};

struct resource 
{
    string name;
    int available;
    sem_t sem;
};

struct taskResource
{
    string name;
    int amount;
    int inUse;
};

struct task 
{
    string name;
    int busyTime;
    int idleTime;
    enum state currentState;
    vector<taskResource> resourcesNeeded;
    bool done = false;
    long totalTime = 0;
    long waitingTime = 0;
    int tid;
};

struct threadArg {
    int taskIndex;
    int NITER;
};


string stateToString(enum state);
void *monitorThreadFunc(void *);
void freeUpSem(int);
void *threadFunc(void *);
task createTask(vector<string>);
taskResource createTaskResource(string, string);
resource createResource(string resourceName, string resourceValue);
void readFileLine(ifstream &file);
vector<string> tokenize(char[MAXLINE]);
