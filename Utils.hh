/*
The MIT License (MIT)

Copyright (c) 2019 Pierre Lindenbaum PhD.

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in
all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
THE SOFTWARE.

*/

#ifndef STRING_UTIL_HH
#define STRING_UTIL_HH

#include <vector>
#include <string>
#include <cctype>
#include <cstring>

class Utils {
    public:

	static std::string& normalize_space(std::string& line);
	static std::string& tr(std::string& line, const char* set,char c2);
	static std::string& trs(std::string& line,char c2);

	static bool isBlank(const std::string& line);
	static bool isBlank(const char* s);
	static bool startsWith(const std::string& line,const char* prefix);
	static bool endsWith(const std::string& line,const char* suffix);
	static std::string& trim(std::string& line);
	static int parseInt(const char* s);
	static std::string niceInt(int i);
	static std::size_t split(char delim,std::string line,std::vector<std::string>& tokens);
	static std::size_t split(char delim,std::string line,std::vector<std::string>& tokens,std::size_t limit);
	static std::string join(const char* delim,const std::vector<std::string>& tokens);
};

#endif
