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
		bool isKeepRunning = true;
		bool Connect(c_char*, c_char*);
		void CloseConnection();
		
		int ssSendBinary(const char*);
		int ssRecvBinary(char*&, int);
		int ssSendStr(const std::string&);
		int ssRecvStr(std::string&, int);
		
		bool ParseCommand(char*&);
		
		bool SendFile(const std::string);
		bool SendInfo();
		bool SendFullInfo();
		void RetrieveFile(u64, c_char, const std::string);
};

#endif
