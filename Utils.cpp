#include <cerrno>
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


bool Utils::isBlank(const std::string& line)
{
for(size_t i=0;i< line.size();i++) {
	if(!isspace(line[i])) return false;
}
return true;
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

bool Utils::startsWith(const std::string& line,const char* suffix) {
	ASSERT_NOT_NULL(suffix);
	size_t len = strlen(suffix);
	if(line.length()< len) return false;
	return line.compare(0,len,suffix)==0;
	}


std::size_t Utils::split(char delim,std::string line,std::vector<std::string>& tokens) {
	tokens.clear();
	size_t prev=0;
	size_t j=line.length();
	while(j>0 && line[j-1]==delim) j--;
	for(size_t i=0;i< j;i++) {
		if(line[i]==delim) {
			tokens.push_back(line.substr(prev,prev-i));
			prev=i+1;
		}
	}
	tokens.push_back(line.substr(prev,prev-j));
	return tokens.size();
	}
