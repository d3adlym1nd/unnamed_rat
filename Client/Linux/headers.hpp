#ifndef __HEADERS
#define __HEADERS

#define _DEBUG  //uncomment this to build "version with verbose output"

#ifdef _DEBUG
#include<iostream>
#endif
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
#include<inttypes.h>
#include<dirent.h>
#include<fcntl.h>
#include<sys/utsname.h>

#include<sys/socket.h>
#include<arpa/inet.h>
#include<netinet/in.h>
#include<netdb.h>

#include<openssl/ssl.h>

#ifdef _DEBUG
#include<openssl/err.h>
#include<errno.h>
extern int errno;
#define error() std::cout<<"Sys ErrorCode ["<<errno<<"] "<<strerror(errno)<<"\nERR_print_errors_fp: "; ERR_print_errors_fp(stderr)
#endif

typedef unsigned long long int u64;
typedef unsigned int u_int;
typedef unsigned char u_char;
typedef const char c_char;

#endif
