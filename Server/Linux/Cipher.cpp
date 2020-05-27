#include "Cipher.hpp"
#include "Misc.hpp"

std::size_t LCipher::b64_encoded_size(std::size_t inlen){
	std::size_t ret = inlen;
	if(inlen % 3 != 0){
		ret += 3 - (inlen % 3);
	}
	ret /= 3;
	ret *= 4;
	return ret;
}

std::size_t LCipher::b64_decoded_size(const char *in){
	std::size_t len, ret, i;
	if(in == nullptr){
		return 0;
	}
	len = Misc::StrLen(in);
	ret = len / 4 * 3;
	for(i = len; i-->0;){
		if(in[i] == '='){
			ret--;
		} else {
			break;
		}
	}
	return ret;
}

int LCipher::b64_valid_char(char c){
    if (c >= '0' && c <= '9'){
		return 1;
	}
    if (c >= 'A' && c <= 'Z'){
		return 1;
	}
    if (c >= 'a' && c <= 'z'){
		return 1;
	}
    if (c == '+' || c == '/' || c == '='){
    	return 1;
	}
    return 0;
}
	
char* LCipher::b64_encode(const char *in, std::size_t len){
	const char b64chars[] = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";
	char *out;
	std::size_t elen, i, j, v;
	if(in == nullptr || len == 0){
		return nullptr;
	}
	elen = b64_encoded_size(len);
	out = new char[elen+1];
	out[elen] = '\0';
	for(i = 0, j = 0; i < len; i += 3, j += 4){
		v = in[i];
		v = i + 1 < len ? v << 8 | in[i+1] : v << 8;
		v = i + 2 < len ? v << 8 | in[i+2] : v << 8;
		out[j]     = b64chars[(v >> 18) & 0x3F];
		out[j + 1] = b64chars[(v >> 12) & 0x3F];
		if(i + 1 < len){
			out[j + 2] = b64chars[(v >> 6) & 0x3F];
		}else{
			out[j + 2] = '=';
		}
		if(i + 2 < len){
			out[j + 3] = b64chars[v & 0x3F];
		}else{
			out[j + 3] = '=';
		}
	}
	return out;
}

int LCipher::b64_decode(const char *in, char*& out, std::size_t outlen){
	int b64_t[] = { 62, -1, -1, -1, 63, 52, 53, 54, 55, 56, 57, 58,
        59, 60, 61, -1, -1, -1, -1, -1, -1, -1, 0, 1, 2, 3, 4, 5,
       	6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16, 17, 18, 19, 20,
        21, 22, 23, 24, 25, -1, -1, -1, -1, -1, -1, 26, 27, 28,
        29, 30, 31, 32, 33, 34, 35, 36, 37, 38, 39, 40, 41, 42,
        43, 44, 45, 46, 47, 48, 49, 50, 51 };
	std::size_t len, i, j;
	int v;
	if(in == nullptr || out == nullptr){
		return 0;
	}
	len = Misc::StrLen(in);
	if(outlen < b64_decoded_size(in) || len % 4 != 0){
		return 0;
	}
	for(i = 0; i < len; i++){
		if(!b64_valid_char(in[i])){
			return 0;
		}
	}
	for(i = 0, j = 0; i < len; i += 4, j += 3){
		v = b64_t[in[i] - 43];
		v = (v << 6) | b64_t[in[i + 1] - 43];
		v = in[i + 2] == '=' ? v << 6 : (v << 6) | b64_t[in[i + 2] - 43];
		v = in[i + 3] == '=' ? v << 6 : (v << 6) | b64_t[in[i + 3] - 43];
			out[j] = (v >> 16) & 0xFF;
		if(in[i + 2] != '='){
			out[j + 1] = (v >> 8) & 0xFF;
		}
		if(in[i + 3] != '='){
			out[j + 2] = v & 0xFF;
		}
	}
	return 1;
}

u_int LCipher::calc(char c, u_int complex = 0){
	u_int tmp = c, count = 0;
	if(tmp<32){return 32;}
	while(1){
		if(count++ == (128+complex)){break;}
		if(tmp++ == 126){ tmp = 32;}
	}
	return tmp;
}

u_int LCipher::r_calc(char c, u_int complex = 0){
	int tmp = c, count = 128+complex;
	if(tmp<32){return 32;}
	while(1){
		if(count-- == 0){break;}
		if(tmp-- == 32){tmp = 126;}
	}
	return tmp;
}

char* LCipher::b64_e(const char* message){
	int len = Misc::StrLen((c_char*)message);
	char *out = b64_encode(message, len);
	return out;
}
	
char* LCipher::b64_d(const char* message){
	std::size_t outlen = b64_decoded_size(message);
	char *output = new char[outlen];
	b64_decode(message, output, outlen);
	return output;
}

int LCipher::BinaryCipher(const char* data, char*& output){
	char *tmp = b64_e(data);
	std::string result = strCipher(std::string(tmp));
	delete[] tmp;
	tmp = nullptr;
	std::size_t rlen = result.length();
	output = new char[rlen + 1];
	if(output == nullptr){
		return 1;
	}
	//strncpy(output, result.c_str(), rlen);
	memcpy(output, result.c_str(), rlen);
	//*output[rlen] = '\0';
	return 0;
}

char* LCipher::BinaryUnCipher(const char* data){
	std::string tmp = strUnCipher(std::string((char *)data));
	char *out = b64_d(tmp.data());
	return out;
}

std::string LCipher::strCipher(const std::string& message){
	std::string final = "";
	u_int tcplx = 0;
	for(char c : message){
		char tmp = calc(c, ++tcplx);
		for(char x : this->strPassword){
			tmp ^= x;
		}
		final += tmp;
	}
	return final;
}

std::string LCipher::strUnCipher(const std::string& message){
	std::string final = "";
	u_int tcplx = 0;
	for(char c : message){
		for(char x : this->strPassword){
			c ^= x;
		}
		final += r_calc(c,++tcplx);
	}
	return final;
}
