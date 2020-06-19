#include "HttpDownload.hpp"
#include "Misc.hpp"

bool Downloader::CheckSslReturnCode(SSL *ssl, int iRet){
	int iTmp = SSL_get_error(ssl, iRet);
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
			#endif
			if(!isRetr){
				Sleep(10000);
				isRetr = true;
			}
			isRetr = false;
			return false;
	}
	return true;
}

SOCKET Downloader::InitSocket(const char* cHostname, const char* cPort){
	struct addrinfo strctAd, *strctP, *strctServer;
	memset(&strctAd, 0, sizeof(strctAd));
	strctAd.ai_family = AF_UNSPEC;
	strctAd.ai_socktype = SOCK_STREAM;
	strctAd.ai_protocol = IPPROTO_TCP;
	strctAd.ai_flags = AI_PASSIVE;
	int iRet = getaddrinfo(cHostname, cPort, &strctAd, &strctServer);
	if(iRet != 0){
		#ifdef _DEBUG
		std::cout<<"Host : "<<cHostname<<":"<<cPort<<" getaddrinfo error: "<<gai_strerror(iRet)<<'\n';
		#endif
		return INVALID_SOCKET;
	}
	for(strctP = strctServer; strctP != nullptr; strctP = strctP->ai_next){
		if((sckDownloadSocket =  WSASocket(strctP->ai_family, strctP->ai_socktype, strctP->ai_protocol, nullptr, 0, 0)) == INVALID_SOCKET){
			#ifdef _DEBUG
			std::cout<<"Socker error\n";
			error();
			#endif
			continue;
		}
		if(WSAConnect(sckDownloadSocket, strctP->ai_addr, strctP->ai_addrlen, 0, 0, 0, 0) != 0){
			#ifdef _DEBUG
			std::cout<<"Error connecting\n";
			error();
			#endif
			continue;
		}
		break;
	}
	if(strctP == nullptr){
		freeaddrinfo(strctServer);
		return INVALID_SOCKET;
	}
	freeaddrinfo(strctServer);
	return this->sckDownloadSocket;
}

bool Downloader::Download(const char* cUrl, std::string& strFile){
	isSSL = false;
	bFlag = false;
	isRetr = false;
	std::vector<std::string> vcUrl, vcUrl2;
	Misc::strSplit(cUrl, '/', vcUrl, 50); //increase this for larger subdirectories or url
	if(vcUrl.size() < 2){
		#ifdef _DEBUG
		std::cout<<"Unable to parse url "<<cUrl<<'\n';
		#endif
		return false;
	}
	
	u64 uliFileSize = 0, uliResponseTotalBytes = 0, uliBytesSoFar = 0;
	int iErr = 0, iBytesWrited = 0, iLen = 0, iBytesReaded = 0;
	char cBuffer[2049], cFileBuffer[2049], cHostname[128], cTmp[128], cRemotePort[7];
	memset(cRemotePort, 0, 7);
	char *cToken = nullptr;
	
	std::size_t iLocation = 0, iNLocation = 0, HeaderEnd = 0;
	std::string strTmpFileName = vcUrl[vcUrl.size()-1], strTmpResponse = "", strTmp = "";
	std::ofstream sFile;
	
	SSL *ssl = nullptr;
	
	if(vcUrl[0] == "https:"){
		isSSL = true;
	}
	
	std::string strHeader = "GET ";
	for(int iIt2 = 3; iIt2<int(vcUrl.size()); iIt2++){
		strHeader.append(1, '/');
		strHeader.append(vcUrl[iIt2]);
	}
	strHeader.append(" HTTP/1.1\r\nHost: ");
	strHeader.append(vcUrl[2]);
	strHeader.append("\r\nConnection: Keep-Alive\r\n\r\n");
	iLen = strHeader.length();
	strncpy(cTmp, vcUrl[2].c_str(), 127);
	cToken = strtok(cTmp, ":");
	if(cToken != nullptr){
		strncpy(cHostname, cToken, 127);
		cToken = strtok(nullptr, ":");
		if(cToken != nullptr){
			strncpy(cRemotePort, cToken, 6);
		}
	}
	
	if(strlen(cHostname) == 0){
		//No port provided on url
		strncpy(cHostname, vcUrl[2].c_str(), 127);
	}
	
	if(strlen(cRemotePort) == 0){
		if(isSSL){
			strncpy(cRemotePort, "443", 4);
		} else {
			strncpy(cRemotePort, "80", 3);
		}
	}
	#ifdef _DEBUG
	std::cout<<"Remote port "<<cRemotePort<<"\nSaving file "<<strTmpFileName<<"\n";
	#endif
	while(1){ //loop until receive 200 ok to procede download
		if(InitSocket(cHostname, cRemotePort) == INVALID_SOCKET){
			bFlag = false;
			break;
		}
		if(isSSL){
			ssl = SSL_new(SSL_CTX_new(TLS_client_method()));
			if(ssl == nullptr){
				#ifdef _DEBUG
				std::cout<<"Error creating ssl object\n";
				#endif
				bFlag = false;
				break;
			}
			SSL_set_fd(ssl, sckDownloadSocket);
			iErr = SSL_connect(ssl);
			if(iErr == -1){
				#ifdef _DEBUG
				std::cout<<"Error establishing SSL connection\n";
				error();
				#endif
				bFlag = false;
				break;
			}
			iBytesWrited = SSL_write(ssl, (const char *)strHeader.c_str(), iLen);
		} else {
			iBytesWrited = send(sckDownloadSocket, strHeader.c_str(), iLen , 0);
		}
		
		if(iBytesWrited > 0){
			memset(cBuffer, 0, 2049);
			if(isSSL){
				iBytesReaded = SSL_read(ssl, cBuffer, 2048);
			} else {
				iBytesReaded = recv(sckDownloadSocket, cBuffer, 2048, 0);
			}
			
			if(iBytesReaded > 0){
				cBuffer[iBytesReaded] = '\0';
				//proceed to check and follow all redirection
				strTmpResponse = cBuffer;
				if(strTmpResponse.find("HTTP/1.1 200 ") != std::string::npos || strTmpResponse.find("HTTP/1.0 200 ") != std::string::npos){
					//ok proceed to download
					memcpy(cFileBuffer, cBuffer, iBytesReaded);
					iLocation = std::string(cBuffer).find("Content-Length: ");
					if(iLocation != std::string::npos){
						iNLocation = std::string(cBuffer).find('\r', iLocation);
						HeaderEnd = std::string(cBuffer).find("\r\n\r\n") + 4;
						if(iNLocation != std::string::npos){
							strTmp = std::string(cBuffer).substr(iLocation + 16, (iNLocation - iLocation) - 16);
							uliFileSize = Misc::StrToUint(strTmp.c_str());
							uliResponseTotalBytes = uliFileSize + HeaderEnd;
							#ifdef _DEBUG
							std::cout<<"File size is "<<uliFileSize<<'\n';
							#endif
						}
					}
					if(uliFileSize == 0){
						#ifdef _DEBUG
						std::cout<<"Unable to retrieve remote filesize\n";
						#endif
						bFlag = false;
						break;
					}
					
					//procede here to download an save file
					//save previous part of file that has been downloaded
					sFile.open(strTmpFileName, std::ios::binary);
					strFile = strTmpFileName;
					if(!sFile.is_open()){
						char *cDummyFile = new char[1024];
						if(GetEnvironmentVariable("TEMP", cDummyFile, 1011) <= 0){
							#ifdef _DEBUG
							std::cout<<"Unable to retrieve TEMP folder path using current directory\n";
							error();
							#endif
							strncpy(cDummyFile, "dummy2.t3mp", 12);
						} else {
							strncat(cDummyFile, "dummy2.t3mp", 12);
						}
						#ifdef _DEBUG
						std::cout<<"Unable to open file "<<strTmpFileName<<"\nOpening dummy file "<<cDummyFile<<"\n";
						error();
						#endif
						sFile.open(cDummyFile, std::ios::binary);
						strFile.erase(strFile.begin(), strFile.end());
						strFile.append(cDummyFile);
						Misc::Free(cDummyFile, 1024);
						if(!sFile.is_open()){
							#ifdef _DEBUG
							std::cout<<"Unable to open dummy file\n";
							error();
							#endif
							if(ssl != nullptr){
								SSL_shutdown(ssl);
								SSL_free(ssl);
								ssl = nullptr;
							}
							if(sckDownloadSocket != INVALID_SOCKET){
								closesocket(sckDownloadSocket);
								sckDownloadSocket = INVALID_SOCKET;
							}
							bFlag = false;
							break;
						}
					}
					sFile.write(&cFileBuffer[HeaderEnd], iBytesReaded - HeaderEnd);
					uliBytesSoFar = iBytesReaded;
					if(uliBytesSoFar >= uliResponseTotalBytes){ //download was made on first recv
						#ifdef _DEBUG
						std::cout<<"Download done\n";
						#endif
						bFlag = true;
						break;
					}
					while(1){
						if(isSSL){
							iBytesReaded = SSL_read(ssl, cFileBuffer, 1024);
						} else {
							iBytesReaded = recv(sckDownloadSocket, cFileBuffer, 1024, 0);
						}
						if(iBytesReaded > 0){
							sFile.write(cFileBuffer, iBytesReaded);
							uliBytesSoFar += iBytesReaded;
							#ifdef _DEBUG
							Misc::ProgressBar(uliBytesSoFar, uliResponseTotalBytes);
							std::fflush(stdout);
							#endif
							if(uliBytesSoFar>=uliResponseTotalBytes){
								bFlag = true;
								goto releaseSSL;
							}
						} else {
							if(isSSL){
								if(!CheckSslReturnCode(ssl, iBytesReaded)){
									bFlag = false;
									break;
								}
								Sleep(1000);
								continue;
							}
							if (iBytesReaded == SOCKET_ERROR) {
								#ifdef _DEBUG
								std::cout<<"Connection closed\n";
								error();
								std::cout<<"\nWSA: "<<WSAGetLastError()<<"\n";
								#endif
								bFlag = false;
								goto releaseSSL;
							}
						}
					}
				} else if(strTmpResponse.find("HTTP/1.1 301 ") != std::string::npos){
					//follow redirection
					iLocation = std::string(cBuffer).find("Location: ");
					if(iLocation != std::string::npos){
						iNLocation = std::string(cBuffer).find('\r', iLocation);
						if(iNLocation != std::string::npos){
							if(ssl != nullptr){
								SSL_shutdown(ssl);
								SSL_free(ssl);
								ssl = nullptr;
							}
							if(sckDownloadSocket != INVALID_SOCKET){
								closesocket(sckDownloadSocket);
								sckDownloadSocket = INVALID_SOCKET;
							}
							strTmp = std::string(cBuffer).substr(iLocation +10, iNLocation - iLocation - 10);
							#ifdef _DefaultConstructibleConcept
							std::cout<<"Redirected to "<<strTmp<<'\n';
							#endif
							Misc::strSplit(strTmp.c_str(), '/', vcUrl2, 50);
							if(vcUrl2.size() < 2){
								#ifdef _DEBUG
								std::cout<<"Unable to parse url: "<<strTmp<<'\n';
								#endif
								bFlag = false;
								break;
							}
							if(vcUrl2[0] == "https:"){
								isSSL = true;
							} else {
								isSSL = false;
							}
							strHeader.erase(strHeader.begin(), strHeader.end());
							strHeader = "GET ";
							for(int iIt3 = 3; iIt3<int(vcUrl2.size()); iIt3++){
								strHeader.append(1, '/');
								strHeader.append(vcUrl2[iIt3]);
							}
							strHeader.append(" HTTP/1.1\r\nHost: ");
							strHeader.append(vcUrl2[2]);
							strHeader.append("\r\nConnection: Keep-Alive\r\n\r\n");
							memset(cTmp, 0, 128);
							strncpy(cTmp, vcUrl2[2].c_str(), 127);
							cToken = strtok(cTmp, ":");
							if(cToken != nullptr){
								memset(cHostname, 0, 128);
								memset(cRemotePort, 0, 7);
								strncpy(cHostname, cToken, 127);
								cToken = strtok(nullptr, ":");
								if(cToken != nullptr){
										strncpy(cRemotePort, cToken, 6);
								}
							}
							if(strlen(cHostname) == 0){
								strncpy(cHostname, vcUrl2[2].c_str(), 127);
							}
							if(strlen(cRemotePort) == 0){
									if(isSSL){
										strncpy(cRemotePort, "443", 4);
									} else {
										strncpy(cRemotePort, "80", 3);
									}
							}
							strTmpFileName.erase(strTmpFileName.begin(), strTmpFileName.end());
							strTmpFileName = vcUrl2[vcUrl2.size()-1];
							strFile = strTmpFileName;
							continue;
						} else {
							#ifdef _DEBUG
							std::cout<<"Unable to parse new location\n";
							#endif
							bFlag = false;
							break;
						}
					} else {
						#ifdef _DEBUG
						std::cout<<"Unable to parse new location\n";
						#endif
						bFlag = false;
						break;
					}
				} else {
					//handle here other codes 404 etc...
					#ifdef _DEBUG
					std::cout<<"Not handled response code\n"<<cBuffer<<'\n';
					#endif
					bFlag = false;
					break;
				}
			} else {
				#ifdef _DEBUG
				std::cout<<"Got no response from server\n";
				error();
				#endif
				bFlag = false;
				break;
			}
		} else {
			#ifdef _DEBUG
			std::cout<<"Unable to send packet\n";
			error();
			#endif
			bFlag = false;
			break;
		}
		
	}
	
	releaseSSL:
	if(sFile.is_open()){
		sFile.close();
	}
	if(ssl != nullptr){
		SSL_shutdown(ssl);
		SSL_free(ssl);
		ssl = nullptr;
	}
	if(sckDownloadSocket != INVALID_SOCKET){
		closesocket(sckDownloadSocket);
		sckDownloadSocket = INVALID_SOCKET;
	}
	return bFlag;
}
