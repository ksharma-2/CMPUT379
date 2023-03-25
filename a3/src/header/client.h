#ifndef CLIENT_H
#define CLIENT_H

#include "common.h"
#include <arpa/inet.h>
#include <vector>
#include <fstream>
#include <thread>

void client(char*, char*, char*, char*);
void processPacket(int, vector<string>, ifstream&);
void getPacket(char[], vector<string>&);
vector<string> readFileLine(ifstream&, int);

void handleDelay(vector<string>);
void handleTime(int);
void handleDelete(int, vector<string>, ifstream&);
void handleGet(int, vector<string>, ifstream&);
void handlePut(int, vector<string>, ifstream&);

#endif