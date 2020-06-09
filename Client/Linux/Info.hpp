#ifndef __INFO
#define __INFO

#include "headers.hpp"

struct sPartition{
	char cPartition[20];
	double dParitionSize;  //size is in GB
};


void Partitions(std::vector<struct sPartition>&);
void Cpu(char*&, char*&);
int Mem();
void Uname(char*& cOutput);
int UserName(char*&);

//  /etc/timezone ?

#endif
