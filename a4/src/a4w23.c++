#include "./header/common.h"

//vector<resource> resources;
vector<task> tasks;
unordered_map<string, resource> resources;
pthread_mutex_t lock;

int main(int argc, char * argv[]){

    char *inputFileName;
    char *monitorTime;
    char *niterStr;
    int NITER;
    ifstream inputFile;
    int fp;
    high_resolution_clock::time_point startTime, endTime;
    milliseconds executionTime;

    startTime = high_resolution_clock::now();

    int tcounter = 0;

    if(argc != 4){
        cout << "Usage: a4w23 inputFile monitorTime NITER" << endl;
        return(-1);
    }

    inputFileName = argv[1];
    monitorTime = argv[2];
    niterStr = argv[3];
    NITER = stoi(niterStr);

    inputFile.open("./" + string(inputFileName));

    readFileLine(inputFile);
    unordered_map<string, resource> localResources = resources;
    cout << "created" << endl;

    struct threadArg *arg = new threadArg[tasks.size()];

    for (int i = 0; i < tasks.size(); i++){
        arg[i].taskIndex = i;
        arg[i].NITER = NITER;
    }

    // pthread_t testThread;

    // if (pthread_create(&testThread, NULL, threadFunc, &arg[0]) < 0){
    //     perror("Pthread creation error");
    //     exit(EXIT_FAILURE);
    // }

    // pthread_join(testThread, NULL);

    pthread_t monitorThread;

    if (pthread_mutex_init(&lock, NULL) != 0) {
        printf("\n mutex init has failed\n");
        return 1;
    }

    pthread_t tids[tasks.size()];
    for (int i = 0; i < tasks.size(); i++){

        if(pthread_create(&tids[i], NULL, threadFunc, (void *)&(arg[i])) < 0){
            perror("Pthread creation");
            exit(EXIT_FAILURE);
        }
    }

    if(pthread_create(&monitorThread, NULL, monitorThreadFunc, monitorTime) < 0){
        perror("Monitor thread creation");
        exit(EXIT_FAILURE);
    }


    for(int i = 0; i < tasks.size(); i++){
        pthread_join(tids[i], NULL);
    }

    pthread_join(monitorThread, NULL);

    pthread_mutex_destroy(&lock);

    printf("System Resources:\n");

    for(auto i:resources){
        printf("\t%s:(Max Available= %d, held= 0)\n", i.second.name.c_str(), i.second.available);
    }

    printf("System Tasks:\n");
    int count = 0;

    for(auto i:tasks){
        printf("[%d] %s (%s, runtime= %ld ms, idleTime= %d):\n\t(tid=%#x)\n", count, i.name.c_str(), stateToString(i.currentState).c_str(), i.totalTime, i.idleTime, i.tid);
        for (auto x:i.resourcesNeeded){
            printf("\t%s: (needed= %d, held= 0)\n", x.name.c_str(), x.amount);
        }
        printf("\t(RUN: %d times, WAIT: %ld msec)\n", NITER, i.waitingTime);
        count++;
    }

    endTime = high_resolution_clock::now();

    executionTime = duration_cast<milliseconds>(endTime - startTime);

    printf("Running time= %ld msec \n", executionTime.count());

}

void *monitorThreadFunc(void *arg){

    int monitorSleep = stoi((char *)arg);
    bool active = true;

    while(active){
        active = false;
        
        vector<string> waitingTasks;
        vector<string> runningTasks;
        vector<string> idleTasks;

        for(int i = 0; i < tasks.size(); i++){
            switch(tasks[i].currentState){
                case WAIT:
                    waitingTasks.push_back(tasks[i].name);
                    break;
                case RUN:
                    runningTasks.push_back(tasks[i].name);
                    break;
                case IDLE:
                    idleTasks.push_back(tasks[i].name);
                    break;
            }
        }
        
        pthread_mutex_lock(&lock);

        cout << "monitor: ";

        printf("[WAIT] ");
        for(auto i:waitingTasks){
            cout << i << " ";
        }
        cout << endl;
        
        printf("[RUN] ");
        for(auto i:runningTasks){
            cout << i << " ";
        }
        cout << endl;

        printf("[IDLE] ");
        for(auto i:idleTasks){
            cout << i << " ";
        }
        cout << endl;

        pthread_mutex_unlock(&lock);

        for(auto i:tasks){
            if (i.done == false){
                active = true;
                break;
            }
        }
        usleep(monitorSleep * 1000);
    }

    pthread_exit(0);
}

void *threadFunc(void *arguments){

    int iterations = 0;
    bool allResources = true;
    bool stillWaiting = false;
    struct threadArg *args = (struct threadArg *)arguments;
    int NITER = args->NITER;
    int taskIndex = args->taskIndex;
    task *currentTask = &tasks[taskIndex];
    currentTask->currentState = WAIT;
    const char *taskName = tasks[taskIndex].name.c_str();
    currentTask->tid = pthread_self();

    long time = 0;
    long waitTime = 0;
    high_resolution_clock::time_point startTime, endTime;
    high_resolution_clock::time_point waitingStart, waitingEnd;
    milliseconds executionTime, waitingTime;

    startTime = high_resolution_clock::now();
    endTime = high_resolution_clock::now();
    executionTime = duration_cast<milliseconds>(endTime - startTime);

    // cout << endl;
    // cout << "Thread created: " << currentTask->name << endl;

    while(iterations < NITER ){
        bool allResources = true;
        // cout << endl;
        // cout << "Thread id: " << tasks[taskIndex].name << endl;
        // cout << "Thread iterartion: " << iterations << endl;

        if(!stillWaiting){
            waitingStart = high_resolution_clock::now();
        }

        currentTask->currentState = WAIT;

        for(auto &i:currentTask->resourcesNeeded){
            for (int x = 0; x < i.amount; x++){
                if(sem_trywait(&resources[i.name].sem) == 0){
                    // cout << "Got semaphore" << endl;
                    i.inUse += 1;
                } else {
                    // cout << "Could not get semaphore" << endl;
                    freeUpSem(taskIndex);
                    allResources = false;
                    stillWaiting = true;
                    break;
                }
            }
            if(!allResources){
                break;
            }
        }
        if(allResources){

            waitingEnd = high_resolution_clock::now();
            waitingTime = duration_cast<milliseconds>(waitingEnd - waitingStart);
            waitTime = waitTime + waitingTime.count();
            stillWaiting = false;

            currentTask->currentState = RUN;
            iterations++;

            endTime = high_resolution_clock::now();
            executionTime = duration_cast<milliseconds>(endTime - startTime);
            time = executionTime.count();

            printf("task: %s (tid= %#lx, iter= %d, time= %ld msec)\n", taskName, pthread_self(), iterations, time);
            usleep(currentTask->busyTime * 1000);
            freeUpSem(taskIndex);
            currentTask->currentState = IDLE;
            usleep(currentTask->idleTime * 1000);
        }
    }
    currentTask->waitingTime = waitTime;
    currentTask->totalTime = time;
    currentTask->done = true;
    // cout << "Thread Exiting: " << currentTask->name << endl;
    pthread_exit(0);
}

string stateToString(enum state stateToString){

    switch (stateToString){
        case WAIT:
            return "WAIT";
            break;
        case RUN:
            return "RUN";
            break;
        case IDLE:
            return "IDLE";
            break;
    }

    return "N/A STATE";

}

void freeUpSem(int taskIndex){

    // cout << "Task free up sem: " << tasks[taskIndex].name << endl;

    for(auto &i:tasks[taskIndex].resourcesNeeded){
        // cout << "Resource: " << i.name << endl;
        for (int x = 0; x < i.inUse; x++){
            // cout << "Giving up sem" << endl;
            if (sem_post(&resources[i.name].sem) < 0){
                perror("semaphore error");
                exit(EXIT_FAILURE);
            }
        }
        i.inUse = 0;
    }
}

void readFileLine(ifstream &file){

    string line;
    vector<string> tokenedLine;
    vector<string> resourceLine;
    int taskCounter = 0;

    if(file.is_open()){
        while(getline(file, line) && !file.eof()){
            if(line[0] == '\n' || line[0] == '#' || line[0] == '\0'){
                continue;
            } else {
                tokenedLine = tokenize(&line[0]);
                if (tokenedLine[0] == "resources"){
                    struct resource newResource;
                    tokenedLine.erase(tokenedLine.begin());

                    vector<string>::iterator ptr;
                    for (ptr = tokenedLine.begin(); ptr != tokenedLine.end(); ptr += 2){
                        newResource = createResource(*ptr, *(ptr + 1));
                        resources[newResource.name] = newResource;
                        sem_init(&resources[newResource.name].sem, 1, resources[newResource.name].available);
                    }
                } 
                else if (tokenedLine[0] == "task") {
                    struct task newTask;
                    tokenedLine.erase(tokenedLine.begin());

                    newTask = createTask(tokenedLine);
                    tasks.push_back(newTask);
                }
            }
        }
    } else {
        perror("File error");
        exit(EXIT_FAILURE);
    }
}

task createTask(vector<string> taskArguments){
    struct task newTask;
    struct taskResource newTaskResource;
    vector<taskResource> taskResources;
    string taskName;
    string busyTime;
    string idleTime;

    taskName = taskArguments[0];
    busyTime = taskArguments[1];
    idleTime = taskArguments[2];

    taskArguments.erase(taskArguments.begin());
    taskArguments.erase(taskArguments.begin());
    taskArguments.erase(taskArguments.begin());


    vector<string>::iterator ptr;
    for (ptr = taskArguments.begin(); ptr != taskArguments.end(); ptr += 2){
        newTaskResource = createTaskResource(*ptr, *(ptr + 1));
        taskResources.push_back(newTaskResource);
    }

    newTask.name = taskName;
    newTask.busyTime = stoi(busyTime);
    newTask.idleTime = stoi(idleTime);
    newTask.resourcesNeeded = taskResources;

    return newTask;
                      
}

taskResource createTaskResource(string resourceName, string resourceNeeded){
    taskResource newResource;
    newResource.name = resourceName;
    newResource.amount = stoi(resourceNeeded);
    newResource.inUse = 0;

    return newResource;
}

resource createResource(string resourceName, string resourceValue){
    resource newResource;
    newResource.name = resourceName;
    newResource.available = stoi(resourceValue);

    return newResource;
}

vector<string> tokenize(char input[MAXLINE]){
    const char* delim = ": ";     // Field seperators for tokenizing
    char* tok = strtok(input, delim);
    vector<string> out;

    while(tok != NULL) {
        out.push_back(tok);
        tok = strtok(NULL, delim);
    }

    return out;
}
