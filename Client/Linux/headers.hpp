#ifndef __HEADERS
#define __HEADERS

#define _DEBUG
//#define _DEBUG_CONNECTION

#include<iostream>
#include<thread>
#include<mutex>
#include<fstream>
#include<sstream>
#include<string>
#include<vector>
#include<cstring>
#include<csignal>
#include<ctime>
#include<unistd.h>

#include<sys/socket.h>
#include<sys/wait.h>
#include<arpa/inet.h>
#include<netinet/in.h>
#include<netdb.h>

#ifdef _DEBUG
#include<errno.h>
extern int errno;
#define error() std::cout<<"Error ["<<errno<<"] "<<strerror(errno)<<'\n'
#endif

typedef unsigned long long int u64;
typedef unsigned int u_int;
typedef unsigned char u_char;
typedef const char c_char;

#endif
