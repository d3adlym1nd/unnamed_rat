#ifndef __COMMANDS
#define __COMMANDS
#include "headers.hpp"

namespace CommandCodes{
	c_char cFileTransferCancel[]    = "f@0";
	c_char cFileTranferBegin[]		= "f@1";
	c_char cReqOS[] 				= "c@0";
	c_char cReqBasicInfo[]			= "i@0";
	c_char cReqFullInfo[]			= "i@1";
	c_char cClose[] 				= "s@0@0";
	c_char cShellError[]			= "x@0";
	c_char cShell[] 				=  "x@";
	c_char cDownload[]				=  "d@";
	c_char cUpload[]				=  "u@";
	c_char cHttpd[]					=  "h@";
}

#endif
