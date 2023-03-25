#include <iostream>
#include <netinet/in.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <unistd.h>
#include <chrono>

#define NCLIENT 3
#define SERVERID 0
// #define PORT 9116
#define NOBJECT 16
#define MAXLINE 128
#define MAXWORD 32
#define OBJECTLINE 80

using namespace std;
using namespace std::chrono;