#ifndef __MISC
#define __MISC
#include "headers.hpp"
namespace Misc{
	u64 StrToUint(const char*);
	u_int StrLen(const char*);
	void strSplit(const std::string&, char, std::vector<std::string>&, int);
	void strReplaceSingleChar(std::string&, char, char);
	u64 GetFileSize(std::string);
}
#endif
