#ifndef __MISC
#define __MISC
#include "headers.hpp"
namespace Misc{
	u_int StrToUint(const char *strString);
	u_int StrLen(const char *strString);
	void strSplit(const std::string& strString, char cDelimiter, std::vector<std::string>& vcOut, int iMax);
	void strReplaceSingleChar(std::string& cBuffer, char cOld, char cNew);
}
#endif
