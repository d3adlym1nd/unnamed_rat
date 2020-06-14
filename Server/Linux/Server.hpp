#ifndef __SERVER
#define __SERVER
#include "headers.hpp"
struct Client_Struct{
	int sckSocket;
	SSL *sslSocket;
	int iID;
	std::string strIP;
	std::string strOS;
	bool isFlag;
	bool isConnected;
};

class Server{
	private:
		std::mutex mtxMutex;
		SSL_CTX *sslCTX = nullptr;
	public:		
		struct Client_Struct *Clients[Max_Clients];
		int sckMainSocket = 0;
		int iClientsOnline = 0;
		u_int uiLocalPort = DefaultPort;
		bool isReceiveThread = false;
		bool isCmdThread = false;
		bool isReadingShell = false;
		Server() : uiLocalPort(DefaultPort) {}
		Server(u_int uiPortNumber) : uiLocalPort(uiPortNumber) {}
		~Server(){
			close(sckMainSocket);
			if(sslCTX != nullptr){
				SSL_CTX_free(sslCTX);
			}
		}
		bool Listen(u_int);
		int WaitConnection(char*& output);
		void thStartHandler();
		void mtxLock();
		void mtxUnlock();
		
		//parsing 
		void ParseClientCommand(const std::string, int);
		void ParseMassiveCommand(const std::string);
		void ParseBasicInfo(char*&, int);
		
		//client operation
		void PrintClientList();
		void NullClients();
		void FreeClient(int);
		void FreeAllClients();
		bool DownloadFile(const std::string, int);
		bool SendFile(const std::string, const std::string, int, char);
		
		//threads
		void threadListener();
		void threadMasterCMD();
		void threadClientPing();
		void threadRemoteCmdOutput(int);
		
		void Help(const std::string, int);
	};
	
//extract from beej guide
void *get_int_addr(struct sockaddr *);
#endif
