#include "headers.hpp"
#include "Client.hpp"
#include "Info.hpp"
#include "Misc.hpp"

void test(){
	char *mem = nullptr, *cpu = nullptr, *cores = nullptr, *username = nullptr;
	Uname(mem);
	Cpu(cpu, cores);
	if(UserName(username) == 0){
		std::cout<<"Username: \t"<<username<<'\n';
	}
	
	std::cout<<"System: \t"<<mem<<'\n';
	std::cout<<"Processor: \t"<<cpu<<'\n';
	std::cout<<"Cpu cores: \t"<<cores<<'\n';
	std::cout<<"RAM: \t\t"<<Mem()<<'\n';
	std::vector<struct sPartition> vcVec;
	Partitions(vcVec);
	std::cout<<"Partitions\n";
	for(int iIt = 0; iIt<int(vcVec.size()); iIt++){
		std::cout<<vcVec[iIt].cPartition<<"\t"<<vcVec[iIt].dParitionSize<<'\n';
	}
	delete[] username;
	username = nullptr;
	delete[] mem;
	mem = nullptr;
	delete[] cpu;
	cpu = nullptr;
	delete[] cores;
	cores = nullptr;
}

int main(){
	test();
	return 0;
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
