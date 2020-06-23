#include "headers.hpp"
#include "Misc.hpp"
#include "Info.hpp"

void Users(std::vector<struct sUsers>& vcOutput){
	LPUSER_INFO_11 lUsers = nullptr;
	LPUSER_INFO_11 lTmpuser = nullptr;
	DWORD dCount = 0, dHints = 0;
	NET_API_STATUS nStatus;
	do{
	nStatus = NetUserEnum(nullptr, 11, 0, (LPBYTE*)&lUsers, MAX_PREFERRED_LENGTH, &dCount, &dHints, 0);
	if ((nStatus == NERR_Success) || (nStatus == ERROR_MORE_DATA)){
        if ((lTmpuser = lUsers) != nullptr){
            for (DWORD i = 0; (i < dCount); i++){
                if (lUsers == NULL){
					#ifdef _DEBUG
					std::cout<<"An access violation has occurred\n";
					error();
					#endif
					break;
                }
                std::wstring st = lTmpuser->usri11_name; 
			    std::string strTmp(st.begin(), st.end());
			    if(strTmp != ""){
					struct sUsers sUser;
					strncpy(sUser.cUsername, strTmp.c_str(), UNLEN);
					if(lTmpuser->usri11_priv == USER_PRIV_ADMIN){
						sUser.isAdmin = true;
				    } else {
						sUser.isAdmin = false;
				    }
					vcOutput.push_back(sUser);
			    }
                lTmpuser++;
            }
        }
    }
	if(lUsers != nullptr){
		NetApiBufferFree(lUsers);
		lUsers = nullptr;
	}
	}while(nStatus == ERROR_MORE_DATA);
	if(lUsers != nullptr){
		NetApiBufferFree(lUsers);
		lUsers = nullptr;
	}
}

void Drives(std::vector<struct sDrives>& vcOutput){
	char cDrives[512];
	memset(cDrives, 0, sizeof(cDrives));
	int iRet = GetLogicalDriveStrings(sizeof(cDrives), cDrives);
	if (iRet > 0) {
		char* p1 = cDrives;
		char* p2;
		while (*p1 != '\0' && (p2 = strchr(p1, '\0')) != nullptr) {
			char cLabel[128]; memset(cLabel, '\0', 128);
			char cType[128]; memset(cType, '\0', 128);
			iRet = GetVolumeInformationA(p1, cLabel, 128, nullptr, nullptr, nullptr, cType, 128);
			if(strlen(cLabel) <= 0){
				UINT dt = GetDriveTypeA(p1);
				switch(dt){
					case 0:
						strncpy(cLabel, "Unknown drive", 14);
						break;
					case 1:
						strncpy(cLabel, "No volume mounted", 18);
						break;
					case 2:
						strncpy(cLabel, "USB Drive", 10);
						break;
					case 3:
						strncpy(cLabel, "Hard Disk", 10);
						break;
					case 4:
						strncpy(cLabel, "Remote Drive", 13);
						break;
					case 5:
						strncpy(cLabel, "CD-ROM", 7);
						break;
					case 6:
						strncpy(cLabel, "RAM Disk", 9);
						break;
					default:
						strncpy(cLabel, "Unknown Drive", 14);
						break;
				}
			}
			struct sDrives sDrive;
			if(iRet != 0){
					__int64 FreeBytesAvaiableToUser, TotalFreeBytes;
					GetDiskFreeSpaceEx(p1, (PULARGE_INTEGER)&FreeBytesAvaiableToUser, (PULARGE_INTEGER)&TotalFreeBytes, nullptr);
					double dFreegigs = (((double)(FreeBytesAvaiableToUser / 1024) / 1024) / 1024);
					double dTotalgigs = (((double)(TotalFreeBytes / 1024) / 1024) / 1024);
					strncpy(sDrive.cLetter, p1, 10);
					strncpy(sDrive.cLabel, cLabel, 50);
					strncpy(sDrive.cType, cType, 20);
					sDrive.dFree = dFreegigs;
					sDrive.dTotal = dTotalgigs;
			} else {
					strncpy(sDrive.cLetter, p1, 10);
					strncpy(sDrive.cLabel, cLabel, 50);
					strncpy(sDrive.cType, "-", 2);
					sDrive.dFree = 0;
					sDrive.dTotal = 0;
			}
			vcOutput.push_back(sDrive);
			p1 = p2 + 1;
		}

	}
	
}


void Cpu(char*& cProcessor, char*& cArch){
	int CPUInfo[4] = {-1};
	char CPUBrandString[100];	
	__cpuid(CPUInfo, 0x80000000);
	unsigned int nExIds = CPUInfo[0];
	memset(CPUBrandString, 0, sizeof(CPUBrandString));
	for (unsigned int i=0x80000000; i<=nExIds; ++i){
		__cpuid(CPUInfo, i);
		if  (i == 0x80000002){
			memcpy(CPUBrandString, CPUInfo, sizeof(CPUInfo));
		} else if(i == 0x80000003){
			memcpy(CPUBrandString + 16, CPUInfo, sizeof(CPUInfo));
		} else if  (i == 0x80000004){
			memcpy(CPUBrandString + 32, CPUInfo, sizeof(CPUInfo));
		}
	}
	std::string strTmp = std::string(CPUBrandString).substr(6, strlen(CPUBrandString) -6); //chop blank space
	cProcessor = new char[strTmp.length()+1];
	memset(cProcessor, 0, strTmp.length()+1);
	strncpy(cProcessor, strTmp.c_str(), strTmp.length());
	
	SYSTEM_INFO sInfo;
	GetNativeSystemInfo(&sInfo);
	switch(sInfo.wProcessorArchitecture){
		case 9:
			cArch = new char[20];
			strncpy(cArch, "x64 (AMD or INTEL)", 19);
			break;
		case 5:
			cArch = new char[5];
			strncpy(cArch, "ARM", 4);
			break;
		case 12:
			cArch = new char[8];
			strncpy(cArch, "ARM64", 7);
			break;
		case 6:
			cArch = new char[21];
			strncpy(cArch, "Intel Itanium-based", 20);
			break;
		case 0:
			cArch = new char[5];
			strncpy(cArch, "x86", 4);
			break;
		default:
			cArch = new char[8];
			strncpy(cArch, "Unknow", 7);
			break;
	}
}

void OS(char*& cOS){
	//os regedit
	HKEY hKey;
	auto ret = RegOpenKeyEx(HKEY_LOCAL_MACHINE,TEXT("SOFTWARE\\Microsoft\\Windows NT\\CurrentVersion"), 0, KEY_QUERY_VALUE, &hKey);
	if (ret != ERROR_SUCCESS){
		#ifdef _DEBUG
		std::cout<<"Ret "<<ret<<"\nError "<<GetLastError()<<"\n";
		#endif
		return;
	}
	LPBYTE lBuffer = (LPBYTE)malloc(50);
	DWORD dLen = 50;
	if(RegQueryValueEx(hKey,"ProductName", nullptr, nullptr, lBuffer, &dLen) == ERROR_SUCCESS){
		cOS = new char[dLen + 1];
		strncpy(cOS, (const char *)lBuffer, dLen);
	} else {
		#ifdef _DEBUG
		std::cout<<"Unable to retrieve product name\n";
		error();
		#endif
		cOS = new char[9];
		strncpy(cOS, "Windows", 8); //change this for another method to retrieve operating system
	}
	free(lBuffer);
	lBuffer = nullptr;
	RegCloseKey(hKey);
}

int Mem(){
	MEMORYSTATUSEX mem;
    mem.dwLength = sizeof(mem);
    int iRet = GlobalMemoryStatusEx(&mem);

    if (iRet == 0) {
        #ifdef _DEBUG
        std::cout<<"Failed to memory status\n";
        error();
        #endif
        return 0;
    }
    iRet = (mem.ullTotalPhys / 1024) / 1024;
    return iRet;
}

void UserName(char*& cOutput){
	cOutput = new char[110 + UNLEN];
	memset(cOutput, 0, 110 + UNLEN);
	char cUser[UNLEN + 1];
	char cMachineName[100];
	DWORD dLen = UNLEN + 1;
	if(GetUserName(cUser, &dLen) != 0){
		strncpy(cOutput, cUser, UNLEN + 1);
	} else {
		strncpy(cOutput, "siseneg", 8);	
	}
	dLen = 100;
	if(GetComputerName(cMachineName, &dLen)){
		strncat(cOutput, "@", 2);
		strncat(cOutput, cMachineName, 100);
	} else {
		#ifdef _DEBUG
		std::cout<<"Unable to retrieve computer name\n";
		error();
		#endif
		strncat(cOutput, "@unnamed", 9);
	}
}

