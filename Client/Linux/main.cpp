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
						usleep(100000);
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
		}
	}
	Misc::Free(cBuffer, 1024);
	delete Cli;
	Cli = nullptr;
	return 0;
}
