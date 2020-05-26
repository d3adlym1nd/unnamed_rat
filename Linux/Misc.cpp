#include "Misc.hpp"

namespace Misc{
	u64 StrToUint(const char *strString){
			u_int uiLen = StrLen(strString);
			u_int uiLen2 = uiLen;
			u64 uiRet = 0;
			for(u_int uiIte0 = 0; uiIte0 < uiLen; uiIte0++){
					u_int uiTlen = 1;
					--uiLen2;
					for(u_int uiIte = 0; uiIte<uiLen2; uiIte++){
						uiTlen *= 10; //decimal  uiTlen *= 8;  octal
					}
					u_int uiT = strString[uiIte0] - 48;
					uiRet += (uiTlen * uiT);
			}
			return uiRet;
	}

	u_int StrLen(const char *strString){
		u_int uiCount = 0;
		while(strString[++uiCount] != '\0');
		return uiCount;
	}
	
	void strSplit(const std::string& strString, char cDelimiter, std::vector<std::string>& vcOut, int iMax){
		int istrLen = strString.length(), iIt = 0, iCounter = 0, iTmp = 0;
		for(; iIt<istrLen; iIt++){
			std::string strTmp = "";
			while(strString[iIt] != cDelimiter && strString[iIt] != '\0'){
				strTmp.append(1, strString[iIt++]);
				iCounter++;
			}
			iCounter = 0;
			vcOut.push_back(strTmp);
			if(++iTmp == iMax){ break; }
			
		}
	}
	
	void strReplaceSingleChar(std::string& cBuffer, char cOld, char cNew){
		u_int uiLen = StrLen(cBuffer.c_str()), iIt = 0;
		for(; iIt<uiLen; iIt++){
			cBuffer[iIt] = cBuffer[iIt] == cOld ? cNew : cBuffer[iIt];
		}
	}
	
	u64 GetFileSize(std::string strFileName){
		std::ifstream strmInputFile(strFileName, std::ios::binary);
		if(!strmInputFile.is_open()){
			return 0;
		}
		std::filebuf *pBuf = strmInputFile.rdbuf();
		u64 uTmp = 0;
		uTmp = pBuf->pubseekoff(0, strmInputFile.end, strmInputFile.in);
		pBuf->pubseekpos(0, strmInputFile.in);
		strmInputFile.close();
		return uTmp;
	}
	
}
