#include "Misc.hpp"

namespace Misc{
	
	void PrintTable(std::vector<std::string>& vHeaders, std::vector<std::string>& vLines, const char cSplitChar){
		char cTmp[4];
		cTmp[0] = cSplitChar;
		cTmp[1] = '-';
		cTmp[2] = '-';
		cTmp[3] = '\0';
		int iMaxSize = 0, iHead = vHeaders.size(), iLine = vLines.size(), iIt = 0, iIt2 = 0, iTmp = 0, iTmp2 = 0;
		std::vector<std::string> vcTmp;
		for(; iIt<iLine; iIt++){
			iTmp = SplitSize(vLines[iIt], cSplitChar);
			iMaxSize = iTmp > iMaxSize ? iTmp : iMaxSize;
			iMaxSize = iHead > iMaxSize ? iHead : iMaxSize;
		}
		for(iIt=iHead; iIt<iMaxSize; iIt++){
			vHeaders.push_back("--");
		}
		for(iIt=0; iIt<iLine; iIt++){
			iTmp = SplitSize(vLines[iIt], cSplitChar);
			while(iTmp++ < iMaxSize){
				vLines[iIt].append(cTmp);
			}
		}
		int iFieldsSize[100][100];
		int iFields[100];
		for(iIt=0; iIt<iMaxSize; iIt++){
			iFields[iIt] = 0;
			iFieldsSize[0][iIt] = vHeaders[iIt].length();
		}
		for(iIt=1, iIt2 = 0; iIt2<iLine; iIt++, iIt2++){
			strSplit(vLines[iIt2], cSplitChar, vcTmp, 100);
			iTmp = vcTmp.size();
			for(iTmp2=0; iTmp2<iTmp; iTmp2++){
				iFieldsSize[iIt][iTmp2] = int(vcTmp[iTmp2].length()); 
			}
			while(iTmp2<iMaxSize){
				iFieldsSize[iIt][iTmp2++] = 2; //default --
			}
		}
		
		for(iIt=0; iIt<int(vHeaders.size()); iIt++){
			for(iIt2=0; iIt2<iMaxSize; iIt2++){
				iFields[iIt2] = iFieldsSize[iIt][iIt2] > iFields[iIt2] ? iFieldsSize[iIt][iIt2] : iFields[iIt2];
			}
		}
		
		std::string strPadding = "", strSolidBorder = " *", strCutBorder = " .";
		for(iIt=0; iIt<int(vHeaders.size()); iIt++){
			strSolidBorder.append(iFields[iIt] + 3, '=');
			strCutBorder.append(iFields[iIt] + 3, '-');
		}
		strSolidBorder.pop_back();
		strCutBorder.pop_back();
		strSolidBorder.append(1, '*');
		strCutBorder.append(1, '.');
		std::cout<<strSolidBorder<<"\n | ";
		
		
		for(iIt=0; iIt<int(vHeaders.size()); iIt++){
			strPadding.erase(strPadding.begin(), strPadding.end());
			iTmp = vHeaders[iIt].length();
			iTmp2 = iFields[iIt] > iTmp ? (iFields[iIt] - iTmp) : (iTmp -iFields[iIt]);
			strPadding.append(iTmp2, ' ');
			std::cout<<vHeaders[iIt]<<strPadding<<" | ";
		}
		std::cout<<"\n"<<strSolidBorder<<"\n";
		for(iIt=0; iIt<iLine; iIt++){
			std::cout<<" | ";
			strSplit(vLines[iIt], cSplitChar, vcTmp, 100);
			for(iIt2=0; iIt2<int(vcTmp.size()); iIt2++){
				strPadding.erase(strPadding.begin(), strPadding.end());
				iTmp = vcTmp[iIt2].length();
				iTmp2 = iFields[iIt2] > iTmp ? (iFields[iIt2] - iTmp) : (iTmp < iFields[iIt2]);
				strPadding.append(iTmp2, ' ');
				std::cout<<vcTmp[iIt2]<<strPadding<<" | ";
			}
			std::cout<<"\n"<<strCutBorder<<"\n";
		}
	}
	
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
	
	int SplitSize(const std::string& strString, char cDelimiter){
		int istrLen = strString.length(), iIt = 0, iCount = 0;
		for(; iIt<istrLen; iIt++){
			while(strString[iIt] != cDelimiter && strString[iIt] != '\0'){
				iIt++;
			}
			iCount++;
		}
		return iCount;
	}
	
	void strSplit(const std::string& strString, char cDelimiter, std::vector<std::string>& vcOut, int iMax){
		if(vcOut.size() > 0){
			vcOut.erase(vcOut.begin(), vcOut.end());
		}
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
	
	void strToLower(std::string& strStr){
		for(u_int iIt=0; iIt<strStr.length(); iIt++){
			strStr[iIt] = (strStr[iIt] >= 65 && strStr[iIt] <= 90) ? (strStr[iIt] + 32) : strStr[iIt];
		}
	}
	
	void ProgressBar(u64 value, u64 total){
        int h = 0, hh = 0;
        char pb[101];
        memset(pb, 0, 101);
        int value2 = ((float)value / (float)total) *100;
        for(h=0; h<50; h++){
                for(hh=h; hh<(value2 / 2); hh++, h++){
                        pb[hh] = '#';
                }
                pb[h] = '_';
        }
        pb[50] = '\0';
        std::cout<<'\r'<<pb<<'['<<value2<<"%]";
	}
	
	void Free(char*& Ptr, std::size_t Size){
		if(Ptr != nullptr){
			Size = Size == 0 ? StrLen(Ptr) : Size;
			memset(Ptr, 0, Size);
			delete[] Ptr;
			Ptr = nullptr;
		}
	}
}
