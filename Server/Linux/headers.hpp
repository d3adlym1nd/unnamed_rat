#ifndef __HEADERS
#define __HEADERS

//Uncomment both to make program beautiful
#define _COLOR
//#define _NOTIFY

//LANG
//#define ES
#define EN

//Termux?
//#define _TERMUX

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
#include<fcntl.h>
#include<sys/socket.h>
#include<arpa/inet.h>
#include<netinet/in.h>
#include<netdb.h>
#include<errno.h>

#ifdef _NOTIFY
//Require libnotify-dev
#include<libnotify/notify.h>
#endif

#include<openssl/ssl.h>
#include<openssl/err.h>

extern int errno;
extern bool bSignalFlag;
#define error() std::cout<<"Error["<<errno<<"] "<<strerror(errno)<<'\n'

#ifdef _COLOR
#define Bold0 "\033[1m"
#define CReset "\033[0m"
#define BrightBlack "\033[90;1m"
#define BrightCyan "\033[96;1m"
#define Yellow "\033[33;1m"
#define Blue "\033[34;1m"
#define Red "\033[31;1m"
#define WhiteBk "\033[30;47;1m"
#define BrightBlackBk "\033[97;100;1m"
#else
#define Bold0 ""
#define CReset ""
#define BrightBlack ""
#define BrightCyan ""
#define Yellow ""
#define Blue ""
#define Red ""
#define WhiteBk ""
#define BrightBlackBk ""
#endif

#define Max_Clients 10 //Increment this for more clients
#define DefaultPort 31337

typedef unsigned long long int u64;
typedef const char c_char;
#endif
