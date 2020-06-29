#ifndef __MISC
#define __MISC
#include "headers.hpp"
namespace Misc{
	void PrintTable(std::vector<std::string>&, std::vector<std::string>&, const char);
	u64 StrToUint(const char*);
	u_int StrLen(const char*);
	int SplitSize(const std::string&, char);
	void strSplit(const std::string&, char, std::vector<std::string>&, int);
	void strReplaceSingleChar(std::string&, char, char);
	void strToLower(std::string&);
	u64 GetFileSize(std::string);
	void ProgressBar(u64 value, u64 total);
	void Free(char*&, std::size_t);
	const char* Msg(int);
}
#endif
