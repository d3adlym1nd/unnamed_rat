#include "HttpDownload.hpp"
#include "Misc.hpp"

int Downloader::InitSocket(const char* cHostname, const char* cPort){
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
		return -1;
	}
	for(strctP = strctServer; strctP != nullptr; strctP = strctP->ai_next){
		if((sckDownloadSocket = socket(strctP->ai_family, strctP->ai_socktype, strctP->ai_protocol)) == -1){
			#ifdef _DEBUG
			std::cout<<"Socker error\n";
			error();
			#endif
			continue;
		}
		if(connect(sckDownloadSocket, strctP->ai_addr, strctP->ai_addrlen) == -1){
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
		return -1;
	}
	freeaddrinfo(strctServer);
	return this->sckDownloadSocket;
}

bool Downloader::Download(const char* cUrl, std::string& strFile){
	std::vector<std::string> vcUrl, vcUrl2;
	Misc::strSplit(cUrl, '/', vcUrl, 50); //increase this for larger subdirectories or url
	if(vcUrl.size() < 2){
		#ifdef _DEBUG
		std::cout<<"Unable to parse url "<<cUrl<<'\n';
		#endif
		return false;
	}
	
	unsigned long int uliFileSize = 0, uliResponseTotalBytes = 0, uliBytesSoFar = 0;
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
	
	while(1){ //loop until receive 200 ok to procede download
		if(InitSocket(cHostname, cRemotePort) == -1){
			bFlag = false;
			break;
		}
		if(isSSL){
			ssl = SSL_new(SSL_CTX_new(TLS_method()));
			if(ssl == nullptr){
				#ifdef _DEBUG
				std::cout<<"Error creating ssl object\n";
				ERR_print_errors_fp(stderr);
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
				strTmpResponse = cBuffer;
				if(strTmpResponse.find("HTTP/1.1 200 ") != std::string::npos || strTmpResponse.find("HTTP/1.0 200 ") != std::string::npos){
					memcpy(cFileBuffer, cBuffer, iBytesReaded);
					iLocation = strTmpResponse.find("filename=");
					if(iLocation != std::string::npos){
						iNLocation = strTmpResponse.find("\r", iLocation);
						if(iNLocation != std::string::npos){
							strTmp = strTmpResponse.substr(iLocation +9, (iNLocation - iLocation) - 9);
							strFile = strTmp;
						}
					}
					if(uliFileSize == 0){
						iLocation = strTmpResponse.find("Content-Length: ");
						if(iLocation != std::string::npos){
							iNLocation = strTmpResponse.find('\r', iLocation);
							if(iNLocation != std::string::npos){
								strTmp = strTmpResponse.substr(iLocation + 16, (iNLocation - iLocation) - 16);
								uliFileSize = Misc::StrToUint(strTmp.c_str());
								uliResponseTotalBytes = uliFileSize + HeaderEnd;
								#ifdef _DEBUG
								std::cout<<"File size is "<<uliFileSize<<'\n';
								#endif
							}
						}
					}
					if(uliFileSize == 0){
						#ifdef _DEBUG
						std::cout<<"Unable to retrieve remote filesize\n";
						#endif
						bFlag = false;
						break;
					}
					
					//save previous part of file that has been downloaded
					sFile.open(strTmpFileName, std::ios::binary);
					strFile = strTmpFileName;
					if(!sFile.is_open()){
						#ifdef _DEBUG
						std::cout<<"Unable to open file "<<strTmpFileName<<"\nOpening dummy file /tmp/dowload.t3mp\n";
						error();
						#endif
						sFile.open("/tmp/download.t3mp", std::ios::binary);
						strFile = "/tmp/download.t3mp";
						if(!sFile.is_open()){
							#ifdef _DEBUG
							std::cout<<"Unable to open dummy file\n";
							error();
							#endif
							close(sckDownloadSocket);
							bFlag = false;
							break;
						}
					}
					HeaderEnd = std::string(cBuffer).find("\r\n\r\n") + 4;
					sFile.write(&cFileBuffer[HeaderEnd], iBytesReaded - HeaderEnd);
					uliBytesSoFar = iBytesReaded;
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
								switch(SSL_get_error(ssl, iBytesReaded)){
									case SSL_ERROR_NONE:
										continue;
										break;
									case SSL_ERROR_ZERO_RETURN:
										#ifdef _DEBUG
										std::cout<<"SSL_ERROR_ZERO_RETURN\n";
										error();
										#endif
										bFlag = false;
										iErr = 1;
										break;
									case SSL_ERROR_WANT_READ:
										#ifdef _DEBUG
										std::cout<<"SSL_ERROR_WANT_READ\n";
										error();
										#endif
										bFlag = false;
										iErr = 1;
										break;	
								}
							}
							if(iErr == 1){
								break;
							}
							if (iBytesReaded == -1) {
								#ifdef _DEBUG
								std::cout<<"Connection closed\n";
								error();
								#endif
								bFlag = false;
								goto releaseSSL;
							}
						}
					}
				} else if(strTmpResponse.find("HTTP/1.1 301 ") != std::string::npos){
					//follow redirection
					iLocation = strTmpResponse.find("Content-Length: ");
					if(iLocation != std::string::npos){
						iNLocation = strTmpResponse.find('\r', iLocation);
						if(iNLocation != std::string::npos){
							strTmp = strTmpResponse.substr(iLocation + 16, (iNLocation - iLocation) - 16);
							uliFileSize = Misc::StrToUint(strTmp.c_str());
							#ifdef _DEBUG
							std::cout<<"File size is "<<uliFileSize<<'\n';
							#endif
						}
					}
					iLocation = std::string(cBuffer).find("Location: ");
					if(iLocation != std::string::npos){
						iNLocation = std::string(cBuffer).find('\r', iLocation);
						if(iNLocation != std::string::npos){
							if(sckDownloadSocket){
								close(sckDownloadSocket);
							}
							if(isSSL){
								SSL_free(ssl);
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
					#ifdef _DEBUG
					std::size_t pos = std::string(cBuffer).find("\r\n");
					std::string strTmp = std::string(cBuffer).substr(0, pos);
					std::cout<<"Not handled response code\n"<<strTmp<<'\n';
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
	if(sckDownloadSocket){
		close(sckDownloadSocket);
	}
	if(isSSL){
		if(ssl != nullptr){
			SSL_free(ssl);
		}
	}
	return bFlag;
}
