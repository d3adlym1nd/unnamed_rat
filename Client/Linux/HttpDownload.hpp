#ifndef __DOWNLOAD
#define __DOWNLOAD

#include "headers.hpp"

class Downloader{
	protected:
		int sckDownloadSocket;
	public:
		bool isSSL = false;
		bool bFlag = false;
		
		int InitSocket(const char*, const char*);
		bool Download(const char*, std::string&);
};

#endif
