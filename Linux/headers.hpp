#ifndef __HEADERS
#define __HEADERS

#include<iostream>
#include<thread>
#include<mutex>
#include<sstream>
#include<string>
#include<vector>
#include<cstring> 			//memcpy strncpy
#include<csignal>
#include<ctime>				//time functions
#include<unistd.h>			//sleep 
#include<fcntl.h>			//for non-blocking sockets
#include<sys/socket.h>
#include<arpa/inet.h>
#include<netinet/in.h>
#include<netdb.h>
#include<errno.h>

//Error message
extern int errno;
#define error() std::cout<<"Error["<<errno<<"] "<<strerror(errno)<<'\n'

//For colored terminal
#define Bold0 "\033[1m"
#define Bold1 "\033[0m"
#define Max_Clients 2
#define DefaultPort 31337

typedef unsigned long long int u64; //big number
typedef unsigned int u_int;
typedef const char c_char;
typedef unsigned char u_char;
#endif

//fcntl(sckSocket, F_SETFL, O_NONBLOCK)
