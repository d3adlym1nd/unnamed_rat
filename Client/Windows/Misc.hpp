#ifndef __MISC
#define __MISC
#include "headers.hpp"
namespace Misc{
	u64 StrToUint(const char*);
	u_int StrLen(const char*);
	void strSplit(const std::string&, char, std::vector<std::string>&, int);
	void strReplaceSingleChar(std::string&, char, char);
	template<typename T, std::size_t N>
	void strToLower(std::string&);
	u64 GetFileSize(std::string);
	bool FileExists(const char*);
	void ProgressBar(u64, u64);
	void Free(char*&, std::size_t);
	bool Execute(const char*, int);
}
#endif
