#ifndef __SERVER
#define __SERVER
#include "headers.hpp"
#include "Cipher.hpp"

struct Client_Struct{
	int sckSocket;
	int iID;
	std::string strIP;
	std::string strOS;
	bool isFlag;
	bool isConnected;
};

class Server{
	private:
		LCipher txtCipher;
		std::mutex mtxMutex;
	public:		
		struct Client_Struct *Clients[Max_Clients];
		int sckMainSocket = 0;
		int iClientsOnline = 0;
		u_int uiLocalPort = DefaultPort;
		bool isReceiveThread = false;
		bool isCmdThread = false;
		bool isReadingShell = false;
		Server() : uiLocalPort(DefaultPort){}
		Server(u_int uiPortNumber) : uiLocalPort(uiPortNumber) {}
		//Server(u_int uiPortNumber, const std::string strPassword) : uiLocalPort(uiPortNumber), txtCipher(new LCipher(strPassword)){}
		~Server(){
			
		}
		int ssSendBinary(int, const char*, int);
		int ssRecvBinary(int, char*&, int);
		int ssSendStr(int, const std::string&);
		int ssRecvStr(int, std::string&, int);
		bool Listen(u_int);
		int WaitConnection(char*& output);
		void thStartHandler();
		void mtxLock();
		void mtxUnlock();
		
		//parsing 
		void ParseClientCommand(const std::string, int);
		void ParseMassiveCommand(const std::string);
		
		//client operation
		void FreeClient(int);
		void FreeAllClients();
		bool DownloadFile(const std::string, int);
		bool SendFile(const std::string, const std::string, int);
	};
	
void threadListener(Server&);
void threadMasterCMD(Server&);
void threadClientPing(Server&);
//extract from beej guide
void *get_int_addr(struct sockaddr *);
#endif
