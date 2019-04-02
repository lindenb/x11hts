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
#include <iostream>
#include <vector>
#include <string>
#include <zlib.h>
#include <cstdlib>
#include <cstdio>
#include <unistd.h>
#include <getopt.h>
#include <cassert>
#include "macros.hh"
#include "AbstractCmdLine.hh"
#include "GZipInputStreamBuf.hh"

using namespace std;

#define PATTERN "__SPLIT__"



class InterleavedFastq:public AbstractCmd
    {
    public:
		InterleavedFastq() {
			AbstractCmd::options.push_back(new Option('m',true,"modulo"));
			AbstractCmd::options.push_back(new Option('d',true,"divider"));
			}
		virtual ~InterleavedFastq() {}
		virtual int doWork(int argc,char** argv);
    };


int InterleavedFastq::doWork(int argc,char** argv) {
    int opt;
    int modulo=0;
    int divider =0;
    string optstr = this->build_getopt_str();
    while ((opt = getopt(argc, argv, optstr.c_str())) != -1) {
	    switch (opt) {
	    case 'h':
		    usage(cout);
		    return 0;
	    case 'm':
		    modulo =atoi(optarg);
		    break;
		case 'd':
		    divider =atoi(optarg);
		    break;
	    case '?':
		    cerr << "unknown option -"<< (char)optopt << endl;
		    return EXIT_FAILURE;
	    default: /* '?' */
		    cerr << "unknown option" << endl;
		    return EXIT_FAILURE;
	    }
	}
    if(modulo<0)  {
	cerr << "Bad value for modulo :" << modulo << endl;
	return EXIT_FAILURE;
    }
    if(divider<=0)  {
		cerr << "Bad value for divider :" << divider << endl;
		return EXIT_FAILURE;
		}
	if(modulo>=divider)  {
		cerr << "modulo>=divider" << endl;
		return EXIT_FAILURE;
		}
    string line;
    int nline = 0;
    long nReads  = 0;
    if(optind+1==argc) {
	    GzipInputStreamBuf gzbuf(argv[optind]);
	    istream in(&gzbuf);
	    while(getline(in,line,'\n')) {
	     if(nReads%divider==modulo) cout << line << endl;
	     nline++;
	     if(nline==4) {
		   nline=0;
		   nReads++;
	   	   }
	     }
	  }
	else if(optind+2==argc) {
	    int side=0;
	    GzipInputStreamBuf gzbuf1(argv[optind]);
	    GzipInputStreamBuf gzbuf2(argv[optind+1]);
	    istream in1(&gzbuf1);
	    istream in2(&gzbuf2);
	    while(getline(side==0?in1:in2,line,'\n')) {
	     if(nReads%divider==modulo) cout << line << endl;
	     nline++;
	     if(nline==4)
	     	{
	     	side=1;
	     	}
	     else if(nline==8) {
		   nline=0;
		   nReads++;
		   side=0;
	   	   }
	     }
	  }
    else {
    	cerr << "Illegal Number of arguments." << endl;
    	return EXIT_FAILURE;
    	}
     if(nline!=0) {
       cerr << "Illegal number of reads  in input !" << endl;
       return EXIT_FAILURE;
   		}
	return 0;
  	}
  	 	
int main(int argc,char** argv)
    {
    InterleavedFastq app;
    return app.doWork(argc,argv);
    }
