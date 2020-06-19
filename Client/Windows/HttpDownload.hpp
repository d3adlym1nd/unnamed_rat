#ifndef __DOWNLOAD
#define __DOWNLOAD

#include "headers.hpp"

class Downloader{
	protected:
		SOCKET sckDownloadSocket = INVALID_SOCKET;
	public:
		bool isSSL = false;
		bool bFlag = false;
		bool isRetr = false;
		bool CheckSslReturnCode(SSL*, int);
		SOCKET InitSocket(const char*, const char*);
		bool Download(const char*, std::string&);
};

#endif
