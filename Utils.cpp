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
#include <cerrno>
#include <sstream>
#include "Utils.hh"
#include "macros.hh"
using namespace std;

std::string& Utils::trim(std::string& line) {
while(!line.empty() && isspace(line[line.length()-1])) {
	line.erase(line.length()-1,1);
	}
while(!line.empty() && isspace(line[0])) {
	line.erase(0,1);
	}
return line;
}

std::string& Utils::tr(std::string& line,const char* set,char c2) {
    for(size_t i=0;i< line.size();i++) {
    	if(strchr(set,line[i])!=NULL) line[i]=c2;
	}
    return line;
    }
std::string& Utils::trs(std::string& line,char c2)
    {
    size_t i=0;
    while(i< line.size()) {
       	if(line[i]==c2 && i+1 < line.size() && line[i+1]==c2)
       	    {
       	    line.erase(i,1);
       	    }
       	else
       	    {
       	    i++;
       	    }
   	}
    return line;
    }


string& Utils::normalize_space(std::string& line) {
    tr(line," \t\n\r",' ');
    trs(line,' ');
    trim(line);
    return line;
}

bool Utils::isBlank(const char* s)
    {
    if(s==NULL) return true;
    size_t i=0;
    while(s[i]!=0) {
	if(!isspace(s[i])) return false;
	++i;
	}
    return true;
    }


bool Utils::isBlank(const std::string& line)
{
return isBlank(line.c_str());
}



/** convert int to string with comma sep */
string Utils::niceInt(int i) {
	ostringstream os;
	os << i;
	string s1(os.str());
	string s2;
	for(size_t k=0; k < s1.length();k++)
		{
		if(k>0 && k%3==0) s2.insert(0,",",1);
		s2.insert(0,s1,(s1.length()-1)-k,1);
		}
	return s2;
	}
string Utils::join(const char* delim,const vector<string>& tokens) {
	ASSERT_NOT_NULL(delim);
	ostringstream os;
	for(size_t i=0;i< tokens.size();i++) {
		if(i>0) os << delim;
		os << tokens[i];
		}
	return os.str();
	}

int Utils::parseInt(const char* s){
	char* p2;
	errno=0;
	long int i = strtol(s,&p2,10);
	if(*p2!=0 || errno!=0) FATAL("Bad number \"" << s << "\". Cannot convert to integer.");
	return (int)i;
	}

bool Utils::startsWith(const std::string& line,const char* prefix) {
	ASSERT_NOT_NULL(prefix);
	size_t len = strlen(prefix);
	if(line.length()< len) return false;
	return line.compare(0,len,prefix)==0;
	}

bool Utils::endsWith(const std::string& line,const char* prefix) {
	ASSERT_NOT_NULL(prefix);
	size_t len = strlen(prefix);
	if(line.length()< len) return false;
	return line.compare(line.length()-len,len,prefix)==0;
	}


std::size_t Utils::split(char delim,std::string line,std::vector<std::string>& tokens) {
	tokens.clear();
	size_t prev=0;
	size_t j=line.length();
	while(j>0 && line[j-1]==delim) j--;
	for(size_t i=0;i< j;i++) {
		if(line[i]==delim) {
			tokens.push_back(line.substr(prev,i-prev));
			prev=i+1;
		}
	}
	tokens.push_back(line.substr(prev,j-prev));
	return tokens.size();
	}

std::size_t Utils::split(char delim,std::string line,std::vector<std::string>& tokens,size_t limit) {
	if(limit<1) FATAL("bad value for limit");
	tokens.clear();
	size_t prev=0;
	size_t j=line.length();
	while(j>0 && line[j-1]==delim) j--;
	for(size_t i=0;i< j && tokens.size() +1 < limit ;i++) {
		if(line[i]==delim) {
			tokens.push_back(line.substr(prev,i-prev));
			prev=i+1;
		}
	}
	tokens.push_back(line.substr(prev,j-prev));
	return tokens.size();
	}
