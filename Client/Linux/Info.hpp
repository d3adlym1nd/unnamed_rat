#ifndef __INFO
#define __INFO

#include "headers.hpp"

struct sPartition{
	char cPartition[20];
	double dParitionSize;  //size is in GB
};

struct sUsers{
	char cUsername[30];
	char cShell[128];
};

void Users(std::vector<struct sUsers>&);
void Partitions(std::vector<struct sPartition>&);
void Cpu(char*&, char*&);
int Mem();
void Uname(char*& cOutput);
int UserName(char*&);

//  /etc/timezone ?

#endif
