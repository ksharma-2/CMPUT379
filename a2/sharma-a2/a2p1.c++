#include <string>
#include <sys/wait.h>
#include <unistd.h>
#include <stdio.h>
#include <iostream>
#include <fstream>
#include <chrono>
#include <pthread.h>
#include <string>

using namespace std;
using namespace std::chrono;

#define MAXLINE 128

int flag = 0;

void * thr_fn(void *arg){

    FILE *fpout;
    string input;

    while (flag != 1){
        cin >> input;
        int length = input.length();
        if(input == "quit"){
            pclose(fpout);
            exit(0);
        }
        char *consoleCommand = new char[length + 1];
        strcpy(consoleCommand, input.c_str());
        fpout = popen(consoleCommand, "r");
        if(!fpout){
            cout << "popen error" << endl;
            break;
        } else {
            char buffer[128];
            while (fgets(buffer, 128, fpout) != NULL) {
                printf("%s", buffer);
            }
        }
    }

    return((void *) 0);

}


int main (int argc, char** argv) {

    if(argc != 4){
        printf("Enter in the following arrangment: a2p1 nLine inputFile delay");
        exit(0);
    }

    int nLine = atoi(argv[1]);
    string inputFileName(argv[2]);
    int delay = atoi(argv[3]);
    int counter = 1;
    string line;

    ifstream inputFile;
    inputFile.open(inputFileName);
    pthread_t thr1;

    while(true){
        for(int i = 0; i < nLine; i++){
            if(inputFile.is_open()){
                inputFile >> line;
                cout << "[" << counter << "]: " << line << endl;
                counter++;
            }
        }
        cout << endl;

        flag = 0;

        int thr, err;

        thr = pthread_create(&thr1, NULL, thr_fn, &delay);
        pthread_detach(thr1);

        if (thr != 0){
            cout << endl;
            cout << "Error creating thread" << endl;
            exit(-1);
        }
        cout << "*** Entering a delay of " << delay << " seconds" << endl;
        string command = "\0";
        cout << endl;
        cout << "User Command: ";
        high_resolution_clock::time_point start = high_resolution_clock::now();
        while(true){
            high_resolution_clock::time_point end = high_resolution_clock::now();
            if (duration_cast<milliseconds>(end-start).count() >= delay){
                flag = 1;
                cout << endl;
                cout << "*** Delay period ended" << endl;
                if (err != 0) {
                    cout << endl;
                    cout << "Error canceling thread" << endl;
                    exit(-1);
                }
                break;
            }
        }
    }

    inputFile.close();

    return 0;
}