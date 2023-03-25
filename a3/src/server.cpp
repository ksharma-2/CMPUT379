#include "./header/server.h"

high_resolution_clock::time_point serverStart;


void server(char * portNumber) {

    cout << "Entered server" << endl;

    int sockfd, port, noReady, N;
    int newSocket[NCLIENT];
    char buffer[MAXLINE];
    struct sockaddr_in server;
    struct pollfd pfd[NCLIENT + 2];
    int addressLength = sizeof(server);
    map<string, packet> objects;
    bool done[3] = {false, false, false};

    const char* serverMessage = "Server successfully received message";

    memset ((char *)&server, 0, sizeof(server));
    port = atoi(portNumber);

    if((sockfd = socket(AF_INET, SOCK_STREAM, 0)) < 0){
        perror("socket failure");
        exit(EXIT_FAILURE);
    } else {
        cout << "Socket created successfully" << endl;
    }

    cout << "Socket created" << endl;

    server.sin_family = AF_INET;
    server.sin_addr.s_addr = htonl(INADDR_ANY);
    server.sin_port = htons(port);

    if (::bind(sockfd, (struct sockaddr*) &server, sizeof(server)) < 0) {
        perror("bind failure");
        exit(EXIT_FAILURE);
    } else {
        cout << "Socket binded successfully" << endl;
    }

    if(listen(sockfd, NCLIENT) < 0){
        perror("listen failure");
        exit(EXIT_FAILURE);
    } else {
        cout << "listen successful" << endl;
    }

    pfd[0].fd = sockfd;
    pfd[0].events = POLLIN;
    pfd[0].revents = 0;

    pfd[1].fd = STDIN_FILENO;
    pfd[1].events = POLLIN;
    pfd[1].revents = 0;

    N = 2;

    serverStart = high_resolution_clock::now();

    while(1){

        noReady = poll(pfd, NCLIENT + 2, 0);

        if (N < NCLIENT + 2 && (pfd[0].revents && POLLIN)){
            //accept a new connection
            if ((newSocket[N - 2] = accept(sockfd, (struct sockaddr*)&server, (socklen_t*)&addressLength)) < 0){
                perror("Failure");
                exit(EXIT_FAILURE);
            } else {
                // cout << "accept successful" << endl;
            }

            pfd[N].fd = newSocket[N - 2];
            pfd[N].events = POLLIN;
            pfd[N].revents = 0;

            N++;
        }

        for(int i = 1; i < N; i++) {
            if (!done[i-1] && (pfd[i].revents && POLLIN)){
                // keyboard is instantiated as the last index of the polling clients
                if(i == 1){
                    // get keyboard input and process
                    char in[MAXLINE];
                    fgets(in, MAXLINE, stdin);
                    string input;
                    input = in;
                    if(strcmp(input.c_str(), "quit\n") == 0){
                        for (int i = 2; i < N + 1; i++){
                            close(newSocket[i]);
                        }
                        close(sockfd);
                        exit(0);
                    } else if (strcmp(input.c_str(), "list\n") == 0){
                        map<string, packet>::iterator it;
                        for(it = objects.begin(); it != objects.end(); it++){
                            printf("(owner = %c, %s)\n", it->second.id, it->first.c_str());
                            for (int i = 0; i < it->second.noOfLines; i++){
                                printf("[%d]: '%s'\n", i, it->second.lines[i].c_str());
                            }
                        }
                    }
                    continue;
                } else {
                    char id[1];
                    if(read(newSocket[i-2], id, 1) == 0){
                        done[i-1] = true;
                        printf("Client %c has disconnected", id[0]);
                        continue;
                    }
                    handlePacket(newSocket[i-2], id, objects);

                }
            }
        }
    }

    for(int i = 0; i < N + 1; i++){
        close(newSocket[i]);
    }

}

void handlePacket(int socket, char id[1], map<string, packet> &objects){
    char packetType[10];
    char object[MAXWORD];

    memset(packetType, 0, 10);
    memset(object, 0, MAXWORD);

    read(socket, packetType, 10);

    if(strcmp(packetType, "gtime") == 0){
        printf("Received (src = %c) GTIME", id[0]);
        // return a message stating the time
        high_resolution_clock::time_point end = high_resolution_clock::now();
        float time = duration_cast<milliseconds>(end-serverStart).count() / 1000.0;
        string msg = "Received (src = 0) (TIME: " + to_string(time) + ")";
        write(socket, msg.c_str(), MAXLINE);
        printf("Transmitted (src = 0) (TIME: %s)", to_string(time).c_str());
    } else if (packetType != "delay") {

    read(socket, object, MAXWORD);

    cout << endl;

    printf("Received (src= %c) (%s, %s)\n", id[0], packetType, object);

    if (string(packetType) == "put"){
        handlePut(socket, id, object, objects);
    } else if (string(packetType) == "get"){
        handleGet(socket, id, object, objects);
    } else if (string(packetType) == "delete"){
        handleDelete(socket, id, object, objects);
    }

    }
}

void handleDelete(int socket, char id[1], char objectName[MAXWORD], map<string, packet> &objects){

    if(objects[string(objectName)].id != atoi(id)){
        write(socket, "Error", MAXLINE);
        printf("Transmitted (src= 0) (ERROR: object does not belong to client)\n");
    } else {
        objects.erase(string(objectName));
        write(socket, "Success", MAXLINE);
        printf("Transmitted (src = 0) OK\n");
    }
}

void handleGet(int socket, char id[1], char objectName[MAXWORD], map<string, packet> &objects){

    char buffer[OBJECTLINE];
    int count = 0;

    if(objects.find(string(objectName)) == objects.end()){
        write(socket, "Received (src= 0) (ERROR: object not found)", OBJECTLINE);
        printf("Transmitted (src= 0) (ERROR: object not found)\\\n\n");
    } else {
        int noLines = objects[string(objectName)].noOfLines;
        write(socket, "Received (src = 0) OK", OBJECTLINE);
        for(int i = 0; i < noLines; i++){
            write(socket, objects[string(objectName)].lines[i].c_str(), OBJECTLINE);
        }
        printf("Transmitted (src= 0) OK\n\n");
    }
    write(socket, "}", OBJECTLINE);
}


void handlePut(int socket, char id[1], char objectName[MAXWORD], map<string, packet> &objects){
    
    char buffer[OBJECTLINE];
    int count = 0;
    bool exists = false;

    while(1){
        read(socket, buffer, OBJECTLINE);
        if(string(buffer) == "{"){
            continue;
        } else if (string(buffer) == "}"){
            break;
        }

        if(objects.find(string(objectName)) == objects.end() && !exists){
            objects[string(objectName)].id = id[0];
        } else {
            if (count == 0){
                exists = true;
            }
        }

        if(!exists){
            objects[string(objectName)].lines[count] = string(buffer);
            printf("[%d]: '%s'\n", count, buffer);
            count++;
        }
    }

    if(exists){
        char buffer[OBJECTLINE] = "Received (src = 0) (ERROR: objects already exists)";
        printf("Transmitted (src= 0) (ERROR: objects already exists)\n");   
        write(socket, buffer, OBJECTLINE);
    } else {
        if (objects[string(objectName)].id == id[0] && count > 0) {
            objects[string(objectName)].noOfLines = count;
            printf("Transmitted (src= 0) OK\n");              //print message to server
            char buffer[OBJECTLINE] = "Received (src= 0) OK";       //send message to client
            write(socket, buffer, OBJECTLINE);
        }   
    }
}