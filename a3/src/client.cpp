#include "./header/client.h"

void client(char* idNumber, char* inputFile, char* serverAddress, char* portNumber){

    int connectionStatus, port, sockfd;
    struct sockaddr_in server;
    char buffer[MAXLINE];
    int addressLength = sizeof(server);
    string connectMessage = "HELLO ";
    string userConnect;
    ifstream file;
    vector<string> packet;

    connectMessage.append(idNumber);

    if ((sockfd = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
        perror("\n Socket create error\n");
        exit(EXIT_FAILURE);
    }

    port = atoi(portNumber);

    server.sin_family = AF_INET;
    server.sin_port = htons(port);


    if (inet_pton(AF_INET, serverAddress, &server.sin_addr) <= 0) {
        perror("Invalid Address");
        exit(EXIT_FAILURE);
    }

    string input;

    while(1){
        cout << "please reply with 'HELLO IDNUMBER' to connect to the server: ";
        getline(cin, userConnect);
        cout << endl;
        if (userConnect == connectMessage){
            break;
        } else {
            cout << "Invalid message entered please try again." << endl;
        }
    }

    if((connect(sockfd, (struct sockaddr*)&server, addressLength)) < 0 ){
        perror("connection error");
        exit(EXIT_FAILURE);
    }

    file.open("./" + string(inputFile));

    while(1){
        packet = readFileLine(file, sockfd);
        if (packet[0] == string(idNumber)){
            processPacket(sockfd, packet, file);
        }
        cout << endl;
    }
}

void processPacket(int sockfd, vector<string> packet, ifstream &file){

    string id = packet[0];
    string packetType = packet[1];

    if (packetType == "delay") {
        handleDelay(packet);
    } else {
        write(sockfd, packet[0].c_str(), 1);        //Sending ID to server
        write(sockfd, packet[1].c_str(), 10);       //Sending packet type to server
        write(sockfd, packet[2].c_str(), MAXWORD);  //Sending object name to server

        if (packetType == "put"){
            handlePut(sockfd, packet, file);
        } else if (packetType == "get"){
            handleGet(sockfd, packet, file);
        } else if (packetType == "delete") {
            handleDelete(sockfd, packet, file);
        } else if (packetType == "gtime"){
            handleTime(sockfd);
        }
    }
}

void handleDelay(vector<string> packet){
    int ms = stoi(packet[2]);
    cout << "*** Entering a delay for " << ms << " milliseconds" << endl;;
    this_thread::sleep_for(milliseconds(ms));
    cout << "*** Exiting delay period" << endl;
    cout << endl;
}

void handleTime(int sockfd){
    char buffer[MAXLINE];

    printf("Transmitted (src = 1) GTIME\n");

    read(sockfd, buffer, MAXLINE);

    printf("%s", buffer);
}

void handleDelete(int sockfd, vector<string> packet, ifstream &file){

    char buffer[MAXLINE];

    printf("Transmitted (src= 1) (DELETE: %s)\n", packet[2].c_str());

    read(sockfd, buffer, MAXLINE);

    if(strcmp(buffer, "Error") == 0){
        printf("Received (src= 0) (ERROR: object does not belong to client)\n");
    } else {
        printf("Received (src= 0) OK\n");
    }

}

void handleGet(int sockfd, vector<string> packet, ifstream &file){

    char buffer[OBJECTLINE];

    printf("Transmitted (src= 1) (GET: %s)\n", packet[2].c_str());

    while(1){
        read(sockfd, buffer, OBJECTLINE);
        if (string(buffer) == "}"){
            break;
        } else {
            printf("%s\n", buffer);
        }
    }
}

void handlePut(int sockfd, vector<string> packet, ifstream &file){

    string line;
    char buffer[OBJECTLINE];
    int count = 0;

    printf("Transmitted (src= 1) (PUT: %s)\n", packet[2].c_str());

    if(file.is_open()) {
        while(getline(file, line) && !file.eof()){
            write(sockfd, line.c_str(), OBJECTLINE);
            if (line == "{"){
                continue;
            }
            if (line == "}"){
                break;
            }
            printf("[%d]: '%s'\n", count, line.c_str());
            count++;
        }
    }

    memset(buffer, 0, 80);

    read(sockfd, buffer, OBJECTLINE);
    printf("%s\n", buffer);

}

vector<string> readFileLine(ifstream &file, int fd){

    string line;
    vector<string> packet;

    if(file.is_open()){
        while(getline(file, line) && !file.eof()){
            if(line[0] == '\n' || line[0] == '#'){
                continue;
            } else {
                getPacket(&line[0], packet);
                return packet;
            }
        }
    } else {
        perror("File error");
        exit(EXIT_FAILURE);
    }

    close(fd);
    exit(0);

}

void getPacket(char input[MAXLINE], vector<string> &out){
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