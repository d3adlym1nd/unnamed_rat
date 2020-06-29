#include "headers.hpp"
#include "Server.hpp"
#include "Misc.hpp"
bool bSignalFlag;

int main(int argc, char **argv){
	if(argc < 2){
			std::cout<<argv[0]<<" port\n";
			return 0;
	}
    const char *cBanner = BrightBlack "\n" \
"                                              __              __\n"\
"  __  ______  ____  ____ _____ ___  ___  ____/ /  _________ _/ /_\n"\
" / / / / __ \\/ __ \\/ __ `/ __ `__ \\/ _ \\/ __  /  / ___/ __ `/ __/\n"\
"/ /_/ / / / / / / / /_/ / / / / / /  __/ /_/ /  / /  / /_/ / /_\n"\
"\\__,_/_/ /_/_/ /_/\\__,_/_/ /_/ /_/\\___/\\__,_/  /_/   \\__,_/\\__/\n\n" CReset\
"               ";

    std::cout<<cBanner;
	#ifdef EN
	std::cout<<"Command-Line Remote Access Tool\n"\
	"                      by " BrightCyan "d3adlym1nd" CReset "\n";
	#endif
	#ifdef ES
	std::cout<<"Herramienta de Acceso Remoto desde linea de comandos\n"\
	"                      por " BrightCyan "d3adlym1nd" CReset "\n";
	#endif
	bSignalFlag = false;
	OpenSSL_add_ssl_algorithms();
	SSL_load_error_strings();

	u_int uiLport = Misc::StrToUint(argv[1]);
	Server *srvServer = new Server(uiLport);
	srvServer->NullClients();
	if(srvServer->Listen()){
			srvServer->thStartHandler();
	}
	delete srvServer;
	srvServer = nullptr;
	BIO_sock_cleanup();
	ERR_free_strings();
	EVP_cleanup();
	return 0;
}
