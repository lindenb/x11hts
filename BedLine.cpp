#include <iostream>
#include <fstream>
#include "BedLine.hh"
#include "Utils.hh"
#include "macros.hh"
using namespace std;

BedCodec::BedCodec() {
}

BedCodec::~BedCodec() {
}

bool BedCodec::is_ignore(const string& line) {
    if(Utils::isBlank(line)) return true;
    if(Utils::startsWith(line, "#")) return true;
    if(Utils::startsWith(line, "browser")) return true;
    if(Utils::startsWith(line, "track")) return true;
    return false;
    }

bool BedCodec::parse(const vector<string>& tokens,std::string* chrom,int* start0,int* end0) {
    if(tokens.size()<3) FATAL("not enough tokens in " << Utils::join("\\t",tokens));
    if(tokens[0].empty()) FATAL("empty chromosome in " << Utils::join("\\t",tokens));
    int start = Utils::parseInt(tokens[1].c_str());
    int end= Utils::parseInt(tokens[2].c_str());
    if(start<0) FATAL("bad negative start in "<< Utils::join("\\t",tokens));
    if(start>end) FATAL("start>end "<< Utils::join("\\t",tokens));
    chrom->assign(tokens[0]);
    *start0=start;
    *end0=end;
    return true;
    }
bool BedCodec::parse(const string& line,std::string* chrom,int* start0,int* end0) {
    std::vector<std::string> tokens;
    Utils::split('\t',line,tokens,4);
    return parse(tokens,chrom,start0,end0);
    }
