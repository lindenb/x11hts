#ifndef STRING_UTIL_HH
#define STRING_UTIL_HH

#include <vector>
#include <string>
#include <cctype>
#include <cstring>

class Utils {
	static bool isBlank(const std::string& line);
	static bool startsWith(const std::string& line,const char* suffix);
	static std::string& trim(std::string& line);
	static int parseInt(const char* s);
	static std::string niceInt(int i);
	static std::size_t split(char delim,std::string line,std::vector<std::string>& tokens);
	static std::string join(const char* delim,const std::vector<std::string>& tokens);
};

#endif
