#include "headers.hpp"
#include "Client.hpp"
#include "Misc.hpp"


int main(){
	signal(SIGPIPE, SIG_IGN);
	SSL_library_init();
	OpenSSL_add_all_algorithms();
	#ifdef _DEBUG
	SSL_load_error_strings();
	#endif
	Client *Cli = new Client;
	char *cBuffer = new char[1024];
	while(Cli->isKeepRunning){
		if(Cli->Connect("127.0.0.1", "31337")){
			#ifdef _DEBUG
			std::cout<<"Connected!!!\n";
			#endif
			if(SSL_write(Cli->sslSocket, "01", 2) > 0){
				while(1){
					int iBytes = SSL_read(Cli->sslSocket, cBuffer, 1023);
					if(iBytes > 2){
						cBuffer[iBytes] = '\0';
						#ifdef _DEBUG_CONNECTION
						std::cout<<"Received\n---------------\n"<<cBuffer<<"\n---------------\n";
						#endif
						if(!Cli->ParseCommand(cBuffer)){
							Cli->CloseConnection();
							Cli->isKeepRunning = false;
							break;
						}
					} else if(iBytes == -1){
						Cli->CloseConnection();
						break;
					}
				}
			} else {
				Cli->CloseConnection();
			}
		} else {
			#ifdef _DEBUG
			sleep(3);
			#else
			sleep(60);                  
			#endif
			break; //remove this
		}
	}
	Misc::Free(cBuffer, 0);
	delete Cli;
	Cli = nullptr;
	return 0;
}
