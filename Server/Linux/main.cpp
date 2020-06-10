#include "headers.hpp"
#include "Server.hpp"
#include "Misc.hpp"

int main(int argc, char **argv){
	if(argc < 2){
			std::cout<<argv[0]<<" port\n";
			return 0;
	}
	SSL_library_init();
	OpenSSL_add_all_algorithms();
	SSL_load_error_strings();
	signal(SIGPIPE, SIG_IGN);
	u_int uiLport = Misc::StrToUint(argv[1]);
	Server *srvServer = new Server(uiLport);
	
	if(srvServer->Listen(10)){
			srvServer->thStartHandler();
	}
	delete srvServer;
	srvServer = nullptr;
	return 0;
}
