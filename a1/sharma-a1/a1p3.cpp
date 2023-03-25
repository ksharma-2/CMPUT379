#include <sys/wait.h>
#include <unistd.h>
#include <stdio.h>
#include <sys/times.h>
#include <iostream>
#include <fstream>
#include <cstring>
#include <vector>

using namespace std;

#define MAXLINE  128
#define MAXWORD  20

// Structure for linking pid and command
struct process {
    pid_t pid;
    string command;
};

int main(int argc, char **argv) {

    struct tms tStart, tEnd;        // tms that holds start and end time
    clock_t start, end;             // start and end 'real' time
    int intArg;                     // Holds either the integer entered (1, 0, or -1)
    char inputLine[MAXLINE];        // Holds each line from the input file
    const char* WSPACE = "\n ";     // Field seperators for tokenizing
    vector<process> processes;      // Holds all the commands

    // Records the starting time
    start = times(&tStart);

    // Records the integer argument
    intArg = atoi(argv[1]);

    // Gets every input from input file
    while(fgets(inputLine, MAXLINE, stdin)){
        // Skip any lines that start with '#' or a newline
        if (inputLine[0] == '#' || inputLine[0] == '\n') {
            continue;
        } else {
            // Fork a child process
            pid_t pid = fork();
            char* args[5];
            // Set the array of args to NULL
            memset(&args, '\0', sizeof(args));
            char* exeFile;
            char* tok = strtok(inputLine, WSPACE);
            int count = 0;
            string command = "";
            exeFile = tok;
            // Tokenize the input line
            while (tok != NULL) {
                args[count] = tok;
                command = command + " " + args[count];
                count++;
                tok = strtok(NULL, WSPACE);
            }
            cout << "print_cmd(): [" << command << "]" << endl;
            // If a child process
            if (pid == 0) {
                // Replace with command line input
                int status = execvp(exeFile, args);
                // If error
                if (status == -1) {
                    // Print error, and exit
                    cout << endl;
                    cout << "Child (" << getpid() << " unable to execute " << command << ")" << endl; 
                    cout << endl;
                    exit(-1);
                }
            } else {
                // Add to list of processes
                struct process p = {pid, command};
                processes.push_back(p);
            }
        }
    }

    int count = 0;

    // Print all proesses
    for (auto& i : processes) {
        cout << count << " [" << i.pid << ": '" << i.command << "']" << endl;
        count++;
    }
    cout << endl;

    // If integer argument entered is 1 wait for any child process to terminate
    if (intArg == 1) {
        int status;
        pid_t dead = waitpid(-1, &status, 0);
        cout << "process (" << dead << ":'" << "') exited (status= " << status << ")" << endl;
    } else if (intArg == -1) { // If integer argument enterted is -1 wait for all child processes to terminate
        for (int i = 0; i < count; i++) {
            int status;
            waitpid(processes[i].pid, &status, 0);
            cout << "process (" << processes[i].pid << ":'" << processes[i].command << "') exited (status= " << status << ")" << endl;
        }
        cout << endl;
    }
    // If user entered integer is 0, do not wait for any child process termination

    end = times(&tEnd);

    // Figure 8.31 of APUE 3/E
    static long clktck = 0;
    if (clktck == 0) {
        clktck = sysconf(_SC_CLK_TCK);
    }

    // Print all times
    cout << endl;
    cout << "real: " << (end - start) / (double) clktck << " sec." << endl;
    cout << "user: " << tEnd.tms_utime / (double) clktck << " sec." << endl;
    cout << "sys: " << tEnd.tms_stime / (double) clktck  << " sec." << endl;
    cout << "child user: " << tEnd.tms_cutime / (double) clktck  << " sec." << endl;
    cout << "child sys: " << tEnd.tms_cstime / (double) clktck  << " sec." << endl;
    cout << endl;

    return 0;

}