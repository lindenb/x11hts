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
#ifndef ABSTRACT_CMD_LINE
#define ABSTRACT_CMD_LINE

#include <string>
#include <vector>
#include <iostream>

class Option
	{
	public:
		int opt;
		bool has_argument;
		std::string description;
		std::string _arg_name;
		std::string _long_opt;
		bool _hidden;
		bool _required;
		Option(int opt,bool has_argument,const char* desc);
		Option* arg(const char* s);
		Option* hidden();
		Option* required();
	};

class AbstractCmd
	{
        protected:
	    static const int OPT_EXIT_FAILURE;
	    static const int OPT_EXIT_SUCCESS;
	    static const int OPT_CONTINUE;
	public:
		std::string app_name;
		std::string app_version;
		std::string app_desc;
		std::vector<Option*> options;
		
		std::string build_getopt_str();
		AbstractCmd();
		virtual ~AbstractCmd();
		virtual int handle_option(int optc);
		virtual void usage_options(std::ostream& out);
		virtual void usage(std::ostream& out);
		virtual int doWork(int argc,char** argv);
		virtual Option* find_opt_by_opt(int c);
	};

#define DEFAULT_HANDLE_OPTION(opt) do { int __choice = this->handle_option(opt);\
	if(__choice  == AbstractCmd::OPT_EXIT_SUCCESS ) return EXIT_SUCCESS;\
        else if(__choice  == AbstractCmd::OPT_EXIT_FAILURE ) return EXIT_FAILURE;\
        else break;\
	} while(0)


#endif

