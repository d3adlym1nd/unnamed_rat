#ifndef __HEADERS
#define __HEADERS
//#define _DEBUG
#include<openssl/bio.h>
#include<openssl/ssl.h>
#ifdef _DEBUG
#include<openssl/err.h>
#include<iostream>
#endif

#include<winsock2.h>
#include<ws2tcpip.h>
#ifndef _WIN32_WINNT
#define _WIN32_WINNT 0x0601
#endif
#include<windows.h>
#include<thread>
#include<mutex>
#include<fstream>
#include<sstream>
#include<string>
#include<vector>
#include<cstring>
#include<ctime>
#include<cstdlib>
#include<intrin.h>
#include<lm.h>
#include<unistd.h>
#include<inttypes.h>
#include<dirent.h>


#ifdef _DEBUG
#define error() std::cout<<"ErrorCode ["<<GetLastError()<<"]\n"
#endif
typedef unsigned long long int u64;
typedef unsigned int u_int;
typedef unsigned char u_char;
typedef const char c_char;

#endif
