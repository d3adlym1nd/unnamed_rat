#ifndef __CIPHER
#define __CIPHER
#include "headers.hpp"

class LCipher{
   private:
	std::size_t b64_decoded_size(const char*);
	int b64_valid_char(char);
	char *b64_encode(const char*, std::size_t);
	int b64_decode(const char*, char*&, std::size_t);
	u_int calc(char, u_int);
	u_int r_calc(char, u_int);
	char *b64_e(const char*, int&);
	char *b64_d(const char*);
   public:
    std::string strPassword;
	LCipher(): strPassword("W3@kYc1PhEr"){} //default password
    LCipher(std::string p): strPassword(p){}
	std::string strCipher(const std::string&);
	std::string strUnCipher(const std::string&);
	int BinaryCipher(const char*, char*&);
	char *BinaryUnCipher(const char*);
	std::size_t b64_encoded_size(std::size_t);
	
	//return size of file ciphered
	u64 GetCipheredFileSize(const std::string);
	//v2
	u64 GetCipheredFileSize2(const std::string, unsigned int);
};

#endif
