#include "AbstractCmdLine.hh"

using namespace std;

Option::Option(char opt,bool has_argument,const char* description):opt(opt),has_argument(has_argument),description(description)
	{
	}

AbstractCmd::AbstractCmd() {
	options.push_back(new Option('h',false,"print help"));
	options.push_back(new Option('v',false,"print version"));
	}
AbstractCmd::~AbstractCmd()
	{
	for(auto opt:options) delete opt;
	}


void AbstractCmd::usage1(std::ostream& out) {
	for(auto opt:options) {
		out << " -" << opt->opt;
		if(opt->has_argument) out << " (arg)";
		out << " " << opt->description << endl; 
		}
	}
void AbstractCmd::usage(std::ostream& out)	{
	usage1(out);
	}
	
int AbstractCmd::doWork(int argc,char** argv) {
	usage(cerr);
	return -1;
	}


std::string AbstractCmd::build_getopt_str() {
	string s;
	for(auto opt:options) {
		s+= opt->opt;
		if(opt->has_argument) s+=":";
		}
	return s;
	}

