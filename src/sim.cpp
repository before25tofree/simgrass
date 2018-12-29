#include <string>
#include "sim.h"

namespace sim{

	std::string encode(const std::string& s){
		static const char *hex = "0123456789abcdef";
		std::string ret;
		int size = (int)s.size();
		for(int i= 0; i<size; i++){
			char c = s[i];
			switch(c){
				case ' ':
					ret.append("\\s");
					break;
				case '\r':
					ret.append("\\r");
					break;
				case '\n':
					ret.append("\\n");
					break;
				case '\t':
					ret.append("\\t");
					break;
				case '\\':
					ret.append("\\\\");
					break;
				default:
					if(c >= '!' && c <= '~'){
						ret.push_back(c);
					}else{
						ret.append("\\x");
						unsigned char d = c;
						ret.push_back(hex[d >> 4]);
						ret.push_back(hex[d & 0x0f]);
					}
					break;
			}
		}
		return ret;
	}

	inline static
		int hex_int(char c){
			if(c >= '0' && c <= '9'){
				return c - '0';
			}else{
				return c - 'a' + 10;
			}
		}

	std::string decode(const std::string& s){
		int size = (int)s.size();
		std::string ret;
		for(int i= 0; i<size; i++){
			char c = s[i];
			if(c != '\\'){
				ret.push_back(c);
			}else{
				if(i >= size - 1){
					continue;
				}
				char c2 = s[++i];
				switch(c2){
					case 's':
						ret.push_back(' ');
						break;
					case 'a':
						ret.push_back('\a');
						break;
					case 'b':
						ret.push_back('\b');
						break;
					case 'f':
						ret.push_back('\f');
						break;
					case 'v':
						ret.push_back('\v');
						break;
					case 'r':
						ret.push_back('\r');
						break;
					case 'n':
						ret.push_back('\n');
						break;
					case 't':
						ret.push_back('\t');
						break;
					case '\\':
						ret.push_back('\\');
						break;
					case 'x':
						if(i < size - 2){
							char c3 = s[++i];
							char c4 = s[++i];
							ret.push_back((char)((hex_int(c3) << 4) + hex_int(c4)));
						}
						break;

					default:
						ret.push_back(c2);
						break;
				}
			}
		}
		return ret;
	}

}; // namespace sim
