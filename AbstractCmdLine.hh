#ifndef ABSTRACT_CMD_LINE
#define ABSTRACT_CMD_LINE

#include <string>
#include <vector>
#include <iostream>

class Option
	{
	public:
		char opt;
		bool has_argument;
		std::string description;
		Option(char opt,bool has_argument,const char* desc);
	};

class AbstractCmd
	{
	public:
		std::vector<Option*> options;
		
		std::string build_getopt_str();
		AbstractCmd();
		virtual ~AbstractCmd();
		virtual void usage1(std::ostream& out);
		virtual void usage(std::ostream& out);
		virtual int doWork(int argc,char** argv);
	};

#endif

