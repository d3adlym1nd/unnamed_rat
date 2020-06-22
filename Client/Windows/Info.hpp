#ifndef __INFO
#define __INFO

#include "headers.hpp"

struct sDrives{
	char cLetter[10];
	char cLabel[50];
	char cType[20];
	double dFree;
	double dTotal;
};

struct sUsers{
	char cUsername[UNLEN + 1];
	bool isAdmin;
};

void Users(std::vector<struct sUsers>&);
void Drives(std::vector<struct sDrives>&);
void Cpu(char*&, char*&);
void OS(char*&);
int Mem();
void UserName(char*&);

#endif
