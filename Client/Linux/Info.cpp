#include "headers.hpp"
#include "Misc.hpp"
#include "Info.hpp"

bool isNormalShell(const std::string strShell){
	const char strShells[6][6] = {"bash", "sh", "zsh", "ksh", "dash", "rbash"};
	for(int iIt = 0; iIt<6; iIt++){
		if(strShell.find(strShells[iIt]) != std::string::npos){
			return true;
		}
	}
	return false;
}

void Users(std::vector<struct sUsers>& vcOutput){
	std::ifstream strmFile("/etc/passwd");
	if(strmFile.is_open()){
		char cBuffer[10240];
		strmFile.read(cBuffer, 10240);
		int iBytes = strmFile.gcount();
		if(iBytes > 0){
			cBuffer[iBytes] = '\0';
			std::vector<std::string> vcTmp;
			Misc::strSplit(cBuffer, '\n', vcTmp, 100);
			for(int iIt = 0; iIt<int(vcTmp.size()); iIt++){
				std::vector<std::string> vcTmp1;
				Misc::strSplit(vcTmp[iIt].c_str(), ':', vcTmp1, 8);
				if(isNormalShell(vcTmp1[vcTmp1.size()-1].c_str())){
					struct sUsers sTmp;
					strncpy(sTmp.cUsername, vcTmp1[0].c_str(), 29);
					strncpy(sTmp.cShell, vcTmp1[vcTmp1.size()-1].c_str(), 127);
					vcOutput.push_back(sTmp);
				}
			}
		}
		strmFile.close();
	}
}

void Partitions(std::vector<struct sPartition>& vcOutput){
	std::ifstream strmFile("/proc/partitions");
	if(strmFile.is_open()){
		char cBuffer[128];
		//First two lines dont needed
		strmFile.getline(cBuffer, 127);
		strmFile.getline(cBuffer, 127);
		int iBytes = 0;
		while(1){
			strmFile.getline(cBuffer, 127);
			if((iBytes = strmFile.gcount()) > 0){
				cBuffer[iBytes] = '\0';
				std::vector<std::string> vcVector;
				Misc::strSplit(std::string(cBuffer), ' ', vcVector, 100);
				if(vcVector.size() > 0){
					struct sPartition sTmp;
					strncpy(sTmp.cPartition, vcVector[vcVector.size()-1].c_str(), 19);
					unsigned long long int uliBytes = Misc::StrToUint(vcVector[vcVector.size()-2].c_str());
					double dSize = ((((uliBytes * 512.00) / 1024.00) / 1024.00) * 2.00) / 1024.00; 
					sTmp.dParitionSize = dSize;
					vcOutput.push_back(sTmp);
				}
			} else {
				break;
			}
		}
	}
}


void Cpu(char*& cProcessor, char*& cCpuCores){
	std::ifstream fCpu("/proc/cpuinfo", std::ios::in);
	if(fCpu.is_open()){
		char *cTmp = new char[4096];
		fCpu.read(cTmp, 4095);
		int iBytes = fCpu.gcount(), iLen = 0;
		if(iBytes > 0){
			cTmp[iBytes] = '\0';
			std::string strTmp = cTmp, strModel = "", strCpuCores = "", strFinal = "";
			std::size_t iLocation = 0, nLocation = 0, aLocation = 0;
			if((iLocation = strTmp.find("model name")) != std::string::npos){
				if((nLocation = strTmp.find('\n', iLocation)) != std::string::npos){
					if((aLocation = strTmp.find(':', iLocation)) != std::string::npos){
						//std::cout<<"Model name : "<<strTmp.substr(aLocation+2, nLocation - aLocation-2)<<'\n';
						strModel = strTmp.substr(aLocation+2, nLocation - aLocation-2);
						if((iLocation = strTmp.find("cpu cores")) != std::string::npos){
							if((nLocation = strTmp.find('\n', iLocation)) != std::string::npos){
								if((aLocation = strTmp.find(':', iLocation)) != std::string::npos){
									//std::cout<<"Cpu Cores : "<<strTmp.substr(aLocation+2, nLocation - aLocation-2)<<'\n';
									strCpuCores = strTmp.substr(aLocation+2, nLocation - aLocation-2);
									strFinal.append(strModel);
									iLen = strFinal.length();
									cProcessor = new char[iLen+1];
									strncpy(cProcessor, strFinal.c_str(), iLen);
									strFinal.erase(strFinal.begin(), strFinal.end());
									strFinal = strCpuCores;
									iLen = strFinal.length();
									cCpuCores = new char[iLen+1];
									strncpy(cCpuCores, strFinal.c_str(), iLen);
								}
							}
						}
					}
				}
			}
		} else {
			#ifdef _DEBUG
			std::cout<<"Unable to read from file\n";
			error();
			#endif
		}
		delete[] cTmp;
		cTmp = nullptr;
	} else {
		#ifdef _DEBUG
		std::cout<<"Unable to open file\n";
		error();
		#endif
	}
}

int Mem(){
	struct dirent **Dirs;
	char cBuffer[20];
	unsigned long long int uliBlockSize = 0;
	int iBytes = 0, iRet = 0, iTmp = 0, iTotal = 0;
	std::string strTmp = "/sys/devices/system/memory/";
	std::ifstream strmFile("/sys/devices/system/memory/block_size_bytes", std::ios::in);
	if(strmFile.is_open()){
		strmFile.read(cBuffer, 19);
		iBytes = strmFile.gcount();
		if(iBytes > 0){
			cBuffer[iBytes] = '\0';
			uliBlockSize = strtoumax(cBuffer, nullptr, 16);
			iRet = scandir(strTmp.c_str(), &Dirs, nullptr, alphasort);
			if(iRet > 0){
				while(iRet--){
					if(strstr(Dirs[iRet]->d_name, "memory") != nullptr){
						std::string strTmp1 = strTmp;
						strTmp1.append(Dirs[iRet]->d_name);
						strTmp1.append("/online");
						std::ifstream strTmp(strTmp1, std::ios::in);
						if(strTmp.is_open()){
							strTmp.read(cBuffer, 3);
							iBytes = strTmp.gcount();
							if(iBytes > 0){
								cBuffer[iBytes] = '\0';
								iTmp += atoi(cBuffer);
							}
							strTmp.close();
						}
					}
					free(Dirs[iRet]);
					Dirs[iRet] = nullptr;
				}
				iTotal = (((uliBlockSize * iTmp) / 1024) / 1024); //return size in MB
				
			} else {
				iTotal = -1;
			}
			free(Dirs);
			Dirs = nullptr;
		} else {
			iTotal =  -1;
		}
	strmFile.close();
	} else {
		iTotal =  -1;
	}
	return iTotal;
}

void Uname(char*& cOutput){
	/*
	 * struct utsname {
               char sysname[];     Operating system name (e.g., "Linux") 
               char nodename[];    Name within "some implementation-defined
                                     network" 
               char release[];     Operating system release (e.g., "2.6.28") 
               char version[];     Operating system version 
               char machine[];     Hardware identifier 
           #ifdef _GNU_SOURCE
               char domainname[];  NIS or YP domain name 
           #endif
	 * */
	int iLen = 0;
	std::string strTmp = "";
	struct utsname sInfo;
	uname(&sInfo);
	strTmp.append(sInfo.sysname);
	strTmp.append(1, ' ');
	strTmp.append(sInfo.version);
	strTmp.append(1, ' ');
	strTmp.append(sInfo.machine);
	iLen = strTmp.length();
	cOutput = new char[iLen+1];
	strncpy(cOutput, strTmp.c_str(), iLen);
}

int UserName(char*& cOutput){
	cOutput = new char[30];
	return getlogin_r(cOutput, 29);
}

