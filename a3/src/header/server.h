#ifndef SERVER_H
#define SERVER_H

#include "common.h"
#include <poll.h>
#include <map>
#include <iterator>

struct packet {

    char id;
    string lines[3];
    int noOfLines;

};

void server(char*);

void handlePacket(int, char[1], map<string, packet>&);
void handleDelete(int, char[1], char[MAXWORD], map<string, packet>&);
void handleGet(int, char[1], char[MAXWORD], map<string, packet>&);
void handlePut(int, char[1], char[MAXWORD], map<string, packet>&);

#endif