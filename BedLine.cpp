#include <iostream>
#include <fstream>
#include "BedLine.hh"
#include "Utils.hh"
#include "macros.hh"
using namespace std;

Interval::Interval():start(0),end(0) {

}
Interval::~Interval() {

}

int Interval::length() const {
	return 1+(end-start);
}

BedCodec::BedCodec() {
}

BedCodec::~BedCodec() {
}

Interval* BedCodec::createInstance() {
	return new Interval;
}

Interval* BedCodec::parse(const std::vector<std::string>& tokens) {
	if(tokens.size()<3) FATAL("not enough tokens in " << Utils::join("\\t",tokens));
	if(tokens[0].empty()) FATAL("empty chromosome in " << Utils::join("\\t",tokens));
	int start = Utils::parseInt(tokens[1].c_str());
	int end= Utils::parseInt(tokens[2].c_str());
	if(start<0) FATAL("bad start in "<< Utils::join("\\t",tokens));
	if(start>end) FATAL("start>end "<< Utils::join("\\t",tokens));
	Interval* rgn = createInstance();
	rgn->contig.assign(tokens[0]);
	rgn->start=start + 1;
	rgn->end=end;
	return rgn;

	}

Interval* BedCodec::parse(const std::string& line) {
	std::vector<std::string> tokens;
	Utils::split('\t',line,tokens);
	return parse(tokens);
	}

Interval* BedCodec::next(std::istream& in) {
	for(;;) {
		string line;
		if(!getline(in,line,'\n')) break;
		if(is_ignore(line)) continue;
		return parse(line);
		}
	return NULL;
}


bool BedCodec::is_ignore(const std::string line) {
	if(Utils::isBlank(line)) return true;
	if(Utils::startsWith(line,"browser")) return true;
	if(Utils::startsWith(line,"#")) return true;
	return false;
}
size_t BedCodec::parseBedFile(const char* fn,std::vector<Interval*>& all) {
	ifstream in(fn,ios::in);
	if(!in.is_open()) FATAL("Cannot open "<< fn << ". "<< strerror(errno));
	std::vector<string> v;
	string line;
	while((getline(in,line,'\n'))) {
		if(is_ignore(line)) continue;
		Utils::split('\t',line,v);
		all.push_back(parse(v));
	}
	in.close();
	return all.size();
}
