// #include "./header/common.h"

#include "./header/server.h"
#include "./header/client.h"

// void client(char *idNumber, char *inputFile, char *severAddress, char *portNumber ){}

int main(int argc, char *argv[]) {

    if (argc != 3 && argc != 6) {
        cout << "Please enter in either of the following formats: " << endl;;
        cout << "Server: a3w23 -s portNumber" << endl;
        cout << "Client: a3w23 -c idNumber inputFile serverAddress portNumber" << endl;
        return(-1);
    }

    if (strcmp(argv[1], (char*)"s") && argc == 3){
        cout << "Entering server" << endl;
        server(argv[2]);
    } else if (strcmp(argv[1], (char*)"c") && argc == 6){
        client(argv[2], argv[3], argv[4], argv[5]);
    } else {
        cout << "Please enter in either of the following formats: " << endl;;
        cout << "Server: a3w23 -s portNumber" << endl;
        cout << "Client: a3w23 -c idNumber inputFile serverAddress portNumber" << endl;
        return(-1);
    }
}