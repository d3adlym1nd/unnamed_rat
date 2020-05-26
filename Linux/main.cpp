#include "headers.hpp"
#include "Commands.hpp"
#include "Misc.hpp"
#include "Server.hpp"
#include "Cipher.hpp"

//Just for test specific section
void test(const char *arg2){
	LCipher test;
	char *output = nullptr;
	char *output2 = nullptr;
	test.BinaryCipher(arg2, output);
	output2 = test.BinaryUnCipher(output);
	std::cout<<"BinaryCipher\n"<<output<<"END\nBinaryUncipher\n"<<output2<<'\n';
	std::cout<<"StrCipher\n"<<test.strCipher(arg2)<<"\nStrUncipher\n"<<test.strUnCipher(test.strCipher(arg2))<<'\n';
	delete[] output2;
	delete[] output;
}



int main(int argc, char **argv){
	//test(argv[1]);
	//return 0;
	if(argc < 2){
			std::cout<<argv[0]<<" port\n";
			return 0;
	}
	struct sigaction act;
	act.sa_handler = SIG_IGN;
	sigemptyset(&act.sa_mask);
	act.sa_flags = SA_RESTART;
	sigaction(SIGPIPE, &act, nullptr);
	
	u_int uiLport = Misc::StrToUint(argv[1]);
	Server *srvServer = new Server(uiLport);
	
	if(srvServer->Listen(10)){
			srvServer->thStartHandler();
	}
	delete srvServer;
	srvServer = nullptr;
	return 0;
}
