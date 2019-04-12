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
#include <cctype>
#include "version.hh"
#include "macros.hh"
#include "AbstractCmdLine.hh"

using namespace std;

Option::Option(int opt,bool has_argument,const char* description):
	opt(opt),
	has_argument(has_argument),
	description(description),
	_arg_name(has_argument?"arg":""),
	_long_opt(""),
	_hidden(false),
	_required(false)
	{
	}

Option* Option::arg(const char* s) {
    _arg_name.assign(s);
    return this;
    }

Option* Option::hidden() {
    _hidden= true;
    return this;
    }
Option* Option::required() {
    _required= true;
    return this;
    }


const int AbstractCmd::OPT_EXIT_FAILURE = -1;
const int AbstractCmd::OPT_EXIT_SUCCESS = -2;
const int AbstractCmd::OPT_CONTINUE = 0;


AbstractCmd::AbstractCmd():app_name("application"),app_version(X11HTS_VERSION) {
	options.push_back(new Option('h',false,"print help"));
	options.push_back(new Option('v',false,"print version"));
	}
AbstractCmd::~AbstractCmd()
	{
	for(auto opt:options) delete opt;
	}

int AbstractCmd::handle_option(int optopt) {
    switch(optopt)
	{
	case 'h':
		usage(cout);
		return  AbstractCmd::OPT_EXIT_SUCCESS;
	case 'v':
		cout << this->app_version << endl;
		return  AbstractCmd::OPT_EXIT_SUCCESS;
	case '?':
		cerr << "unknown option -"<< (char)optopt << endl;
		return AbstractCmd::OPT_EXIT_FAILURE;
	default: /* '?' */
		cerr << "unknown option" << endl;
		return AbstractCmd::OPT_EXIT_FAILURE;
	}
    }



void AbstractCmd::usage_options(std::ostream& out) {
	for(auto opt:options) {
		if(opt->_hidden) continue;
		out << " ";
		if(isprint(opt->opt)) {
		    out << "-" << (char)opt->opt;
		    }
		if(opt->has_argument) out << " (" << opt->_arg_name << ")";
		out << " " << opt->description ;
		if(opt->_required) out << " [REQUIRED]";
		out << endl;
		}
	}
void AbstractCmd::usage(std::ostream& out) {
	out << app_name << endl;
	out << "Version: " << app_version << endl;
	out << "Compilation: " << __DATE__ << " at " __TIME__ << endl;
	out << app_desc << endl;

	out << "## Options" << endl << endl;
	usage_options(out);
	out << endl;
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

Option* AbstractCmd::find_opt_by_opt(int c) {
    for(auto opt:options) {
	if(opt->opt == c) return opt;
	}
    FATAL("Illegal state: cannot find option -"<< (char)c);
    }
