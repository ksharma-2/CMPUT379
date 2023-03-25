#include <iostream>
#include <stdio.h>
#include <string.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>
#include <fstream>
#include <vector>
#include <map>
#include <chrono>
#include <poll.h>
#include <thread>
#include <iterator>

using namespace std;
using namespace std::chrono;

#define MAXLINE 128
#define NCLIENT 3

char const *fifo_0_1_path = "./fifo-0-1";
char const *fifo_0_2_path = "./fifo-0-2";
char const *fifo_0_3_path = "./fifo-0-3";
char const *fifo_1_0_path = "./fifo-1-0";
char const *fifo_2_0_path = "./fifo-2-0";
char const *fifo_3_0_path = "./fifo-3-0";

// Tokenizes the input and seperates into arguments
void encode(char input[MAXLINE], vector<string> &out){
    const char* delim = "# ";     // Field seperators for tokenizing
    char* tok = strtok(input, delim);
    char* args[3];
    int count = 0;

    while(tok != NULL) {
        args[count] = tok;
        out.push_back(args[count]);
        count++;
        tok = strtok(NULL, delim);
    }
}

// Processes the command depending on the packet type
// Returns true or false depending on if the command was successful or not
bool command(string id, string packetType, string object, map<string, string> &objectList){

    // put packettype return and error checking
    if(packetType == "put"){
        if(objectList.find(object) == objectList.end()){
            objectList[object] = id;
            return true;
        } else {
            return false;
        }
    // get packettype return and error checking
    } else if (packetType == "get"){
        if(objectList.find(object) == objectList.end()){
            return false;
        } else {
            return true;
        }
    // delete packettype return and error checking
    } else if (packetType == "delete"){
        if(objectList[object] != id){
            return false;
        } else {
            objectList.erase(object);
            return true;
        }
    }

    return false;

}

// processes list or quit commands from the server
void serverInput(string input, map<string, string> objectList){

    // list command
    if (input == "list\n"){
        // display a list of the objects and their owners
        cout << "Object Table" << endl;
        map<string, string>::iterator it;
        for(it = objectList.begin(); it != objectList.end(); it++){
            cout << "(owner= " << it->second << ", name= " << it->first << ")" << endl;
        }
    // quit command
    } else if (input == "quit\n"){
        // closes the server
        cout << "quitting" << endl;
        exit(0);
    } else {
        cout << "invalid input" << endl;
    }
}

void server(){

    // initialization of variables
    int pipe_read[NCLIENT];
    int pipe_write;
    fd_set readfd;

    high_resolution_clock::time_point serverStart = high_resolution_clock::now();

    map<string, string> objectList;
    map<string, string> errorType;

    // Adding errors depending on the type of packet
    errorType["get"] = "Received (src= server) (ERROR: object not found)";
    errorType["put"] = "Received (src= server) (ERROR: object already exists)";
    errorType["delete"] = "Received (src= server) (ERROR: client not owner)";

    struct pollfd polling_fds[NCLIENT + 1];

    //Opening the pipes
    for(int i = 0; i < NCLIENT; i++){
        char fifoReadPath[MAXLINE];
        snprintf(fifoReadPath, MAXLINE, "fifo-%d-0", i+1);
        pipe_read[i] = open(fifoReadPath, O_RDONLY | O_NONBLOCK);

        //Error opening pipes
        if (pipe_read[i] == -1){
            exit(-1);
        }

        polling_fds[i].fd = pipe_read[i];
        polling_fds[i].events = POLLIN;

    }

    // Adding keyboard to the polling file descriptions
    polling_fds[NCLIENT].fd = STDIN_FILENO;
    polling_fds[NCLIENT].events = POLLIN;

    // infinite loop
    while(1){

        // see how many are ready
        int noReady;
        if((noReady = poll(polling_fds, NCLIENT + 1, -1)) == -1){
            // Error checking poll
            exit(-1);
        }

        // go through each polling client
        for(int i = 0; i < NCLIENT + 1; i++){
            if(polling_fds[i].revents & POLLIN) {
                
                // keyboard is instantiated as the last index of the polling clients
                if(i == NCLIENT){
                    // get keyboard input and process
                    char in[MAXLINE];
                    fgets(in, MAXLINE, stdin);
                    string input;
                    input = in;
                    serverInput(input, objectList);
                    continue;
                }

                // Read data
                char message[MAXLINE];
                vector<string> packet;
                read(pipe_read[i], message, MAX_INPUT);

                // ignore lines that start with # and newline
                if(message[0] != '#' && message[0] != '\n'){
                    // seperate message into arguments
                    encode(message, packet);
                    string id = packet[0];
                    string packetType = packet[1]; 
                    int pipeId = stoi(id);
                    char fdToWritePath[MAXLINE];
                    // grab the appropriate fifo
                    snprintf(fdToWritePath, MAXLINE, "./fifo-0-%d", pipeId);
                    pipe_write = open(fdToWritePath, O_WRONLY);

                    // check what the client is asking for
                    if(packetType == "gtime"){
                        // return a message stating the time
                        high_resolution_clock::time_point end = high_resolution_clock::now();
                        float time = duration_cast<milliseconds>(end-serverStart).count() / 1000.0;
                        string msg = "Received (src = server) (TIME: " + to_string(time) + ")";
                        char *message = (char *)msg.data();
                        write(pipe_write, message, MAXLINE);
                        close(pipe_write);
                        cout << "Received (src= client:" << pipeId << ") " << packetType << endl;
                        cout << "Transmitted (src= server) (TIME: " << time << ")" << endl;

                    } else {
                        // check the received packettype and process
                        // if valid return a valid message, else return error with appropriate error message
                        string objectName = packet[2];
                        cout << "Received (src= client:" << pipeId << ") ( " << packetType << ": " << objectName << ")" << endl;
                        if(command(id, packetType, objectName, objectList) == true){
                            write(pipe_write, "Received (src = server) OK", MAXLINE);
                            close(pipe_write);
                            cout << "Transmitted (src= server) OK" << endl;
                            cout << endl;
                        } else {
                            write(pipe_write, errorType[packetType].data(), MAXLINE);
                            close(pipe_write);
                            cout << "Transmitted (src= server) " << errorType[packetType] << endl;
                            cout << endl;
                        }
                    }

                }               
            }
        }

    }
}

void client(string id, string inputFile) {

    // initalization
    ifstream file;
    string line;

    int fdRead, fdWrite;

    // open specified input file
    file.open("./" + inputFile);

    while(1){
        // check if file was opened
        if(file.is_open()){
            while(getline(file, line)){
                // skip newlines
                if (line[0] == '\n' || line == ""){
                    continue;
                }
                // process message line
                char transmitMessage[MAXLINE];
                vector<string> packet;
                char encodedData[MAXLINE];
                strcpy(encodedData, line.c_str());
                encode(encodedData, packet);

                string commandID = packet[0];
                string packetType = packet[1];
                
                // only send messages that have an id matching the client
                if (id == commandID){
                    
                    // check if packet or delay
                    if(packetType == "delay"){
                        int ms = stoi(packet[2]);
                        cout << "*** Entering a delay for " << ms << " milliseconds" << endl;;
                        this_thread::sleep_for(milliseconds(ms));
                        cout << "*** Exiting delay period" << endl;
                        cout << endl;
                        continue;
                    }

                    // send the packet to the server
                    
                    char fdToWritePath[MAXLINE];
                    snprintf(fdToWritePath, MAXLINE, "./fifo-%s-0", commandID.c_str());

                    fdWrite = open(fdToWritePath, O_WRONLY);
                    write(fdWrite, line.c_str(), MAXLINE);
                    close(fdWrite);

                    //output the appropriate message depending on packettype
                    if(packetType == "gtime"){
                        cout << "Transmitted (src= " << commandID <<  ") " << packetType << endl;
                    } else if (packetType == "delay"){
                        //TODO
                    } else {
                        string objectName = packet[2];
                        cout << "Transmitted (src= " << commandID << ") (" << packetType << ": " << objectName << ")" << endl;
                    }

                    // read the message returned from the server and output
                    char fdToReadPath[MAXLINE];
                    snprintf(fdToReadPath, MAXLINE, "./fifo-0-%s", commandID.c_str());

                    fdRead = open(fdToReadPath, O_RDONLY);
                    read(fdRead, transmitMessage, MAXLINE);
                    close(fdRead);
                    cout << transmitMessage << endl;
                    cout << endl;
                } else {
                    continue;
                }
            }
        } else {
            cout << "File is not open" << endl;;
        }
    }
}

int main (int argc, char** argv) {


    // check if the file was opened with valid number of arguments
    if (argc != 2 && argc != 4){
        cout << "Enter in either of the following formats: " ;
        cout << "% a2p2 -s or % a2p2 -c idNumber inputFile" << endl;
        exit(0);
    }
    cout << argv[1] << endl;;

    // check if the file was opened with valid arguments based on server or client open
    if(argc == 2 && strcmp(argv[1], "-s") == 0){
        server();
    } else if (argc == 4 && strcmp(argv[1], "-c") == 0){
        string id = argv[2];
        string inputFile = argv[3];
        client(id, inputFile);
    } else {
        exit(-1);
    }

    return 0;

}
