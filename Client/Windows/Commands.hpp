#ifndef __COMMANDS
#define __COMMANDS

namespace CommandCodes{
	c_char cFileTransferCancel[]    = "f@0";
	c_char cFileTransferBegin[]		= "f@1";
	c_char cFileError[]				= "f@0";
	c_char cFileSize[]				=  "01";
	c_char cReqOS[] 				= "c@0";
	c_char cReqInfo[]				=  "i";
	c_char cClose[] 				= "s@0";
	c_char cShellRunning[]			= "x@1";
	c_char cShellError[]			= "x@0";
	c_char cShellEnd[]				= "x@2";
	c_char cShell[] 				=  "x";
	c_char cDownload[]				=  "d";
	c_char cUpload[]				=  "u";
	c_char cHttpd[]					=  "h";
}

#endif
