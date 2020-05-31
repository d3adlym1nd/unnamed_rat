#include "headers.hpp"
#include "Client.hpp"

#include "Misc.hpp"
/*void test(const char *arg2){
	LCipher test;
	char *output = nullptr;
	char *output2 = nullptr;
	test.BinaryCipher(arg2, output);
	output2 = test.BinaryUnCipher(output);
	std::cout<<"BinaryCipher\n"<<output<<"END\nBinaryUncipher\n"<<output2<<'\n';
	std::cout<<"StrCipher\n"<<test.strCipher(arg2)<<"\nStrUncipher\n"<<test.strUnCipher(test.strCipher(arg2))<<'\n';
	std::cout<<"BinaryCipher size: "<<Misc::StrLen(output)<<'\n';
	delete[] output2;
	delete[] output;
}*/

void test(const char *argv){
	LCipher test;
	std::cout<<"Size normal is     : "<<Misc::GetFileSize(argv)<<'\n';
	std::cout<<"Encoded size v1 is : "<<test.GetCipheredFileSize(argv)<<'\n';
	std::cout<<"Encoded size v2 is : "<<test.GetCipheredFileSize2(argv, 1024)<<'\n';
}

int main(/*int argc, char **argv*/){
	//test(argv[1]);
	//return 0;
	signal(SIGPIPE, SIG_IGN);
	Client *Cli = new Client;
	char *cBuffer = nullptr;
	while(Cli->isKeepRunning){
		if(Cli->Connect("127.0.0.1", "31337")){
			#ifdef _DEBUG
			std::cout<<"Connected!!!\n";
			#endif
			
			if(Cli->ssSendBinary("01") > 0){
				while(1){
					int iBytes = Cli->ssRecvBinary(cBuffer, 1024);
					if(iBytes > 2){
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
					} else {
						Misc::Free(cBuffer, 0);
					}
				}
				Misc::Free(cBuffer, 0);
			} else {
				Cli->CloseConnection();
			}
		} else {
			#ifdef _DEBUG
			sleep(3);
			#else
			sleep(60);                  
			#endif
			break;
		}
	}
	Misc::Free(cBuffer, 0);
	delete Cli;
	Cli = nullptr;
	return 0;
}
