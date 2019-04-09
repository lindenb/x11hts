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
#include "Utils.hh"

using namespace std;

#define DEFAULT_BUFFER_SIZE 5000000

class InterleavedFastq:public AbstractCmd
    {
    public:
	    InterleavedFastq();
	    virtual ~InterleavedFastq();
	    virtual int doWork(int argc,char** argv);
    };

InterleavedFastq::InterleavedFastq() {
    app_desc.assign("Generate a intervleaved fastq output by printing the fastq record of an input of fastq 'modulo' every 'd' record");

    Option* opt = new Option('m',true,"modulo. Print the fastq if num-fastq%dividuer == modulo");
    opt->arg("integer")->required();
    options.push_back(opt);
    opt = new Option('d',true,"divider");
    opt->arg("integer")->required();
    options.push_back(opt);

    opt = new Option('B',true,"buffer size [5000000]");
    opt->arg("integer")->required();
    options.push_back(opt);
    }

InterleavedFastq::~InterleavedFastq() {

}
#define MUST_READ(IN,L) \
	if(!getline(IN, L,'\n')) FATAL("line missing in fastq " << L1 << ".")

#define WRITE_LINE(L) cout.write(L.data(),L.size());cout.put('\n')

int InterleavedFastq::doWork(int argc,char** argv) {
    int opt;
    long modulo=-1;
    long divider =0;
    size_t buffer_size= DEFAULT_BUFFER_SIZE;

    string optstr = this->build_getopt_str();
    while ((opt = ::getopt(argc, argv, optstr.c_str())) != -1) {
	    switch (opt) {
	    case 'h': usage(cout); return 0;
	    case 'v': cout << app_version << endl; return 0;
	    case 'm':
		    modulo = (long)Utils::parseInt(optarg);
		    break;
		case 'd':
		    divider = (long)Utils::parseInt(optarg);
		    break;
		case 'B':
		    buffer_size = (size_t)Utils::parseInt(optarg);
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
	    cerr << "Bad arguments : modulo>=divider" << endl;
	    return EXIT_FAILURE;
	    }
    string L1;
    string L2;
    string L3;
    string L4;
    ios::sync_with_stdio(false);
    char* outbuf=new char[buffer_size];
    std::cout.rdbuf()->pubsetbuf(outbuf, buffer_size);
    long nReads  = 0;
    if(optind==argc || optind+1==argc) {
	    //single end
	    GzipInputStreamBuf* buf =
		    (optind==argc?
		     new GzipInputStreamBuf(fileno(stdin),buffer_size):
		     new GzipInputStreamBuf(argv[optind],buffer_size)
		     );
            std::istream in(buf);
	    for(;;) {
		if(!getline(in,L1,'\n')) break;
		MUST_READ(in,L2);
		MUST_READ(in,L3);
		MUST_READ(in,L4);

		if(nReads%divider==modulo) {
		    WRITE_LINE(L1);
		    WRITE_LINE(L2);
		    WRITE_LINE(L3);
		    WRITE_LINE(L4);
		    }
		nReads++;
		}
	   cout.flush();
	   delete buf;
	  }
	else if(optind+2==argc) {
	    //paired end
	   GzipInputStreamBuf* buf1 =  new GzipInputStreamBuf(argv[optind  ],buffer_size);
	   GzipInputStreamBuf* buf2 =  new GzipInputStreamBuf(argv[optind+1],buffer_size);	    
		istream in1(buf1);
		istream in2(buf2);
	    string L5;
	    string L6;
	    string L7;
	    string L8;

	    for(;;) {
		if(!getline(in1, L1,'\n')) {
		    if(getline(in2, L5,'\n')) {
			FATAL("extra read in "<< argv[optind+1]);
		        }
		    break;
		    }
		MUST_READ(in1,L2);
		MUST_READ(in1,L3);
		MUST_READ(in1,L4);
		MUST_READ(in2,L5);
		MUST_READ(in2,L6);
		MUST_READ(in2,L7);
		MUST_READ(in2,L8);
		if(nReads%divider==modulo) {
		    WRITE_LINE(L1);
		    WRITE_LINE(L2);
		    WRITE_LINE(L3);
		    WRITE_LINE(L4);
		    WRITE_LINE(L5);
		    WRITE_LINE(L6);
		    WRITE_LINE(L7);
		    WRITE_LINE(L8);
		    }
	     nReads++;
	     }
	   delete buf1;
	   delete buf2;
	    cout.flush();
	  }
    else {
    	cerr << "Illegal Number of arguments." << endl;
    	return EXIT_FAILURE;
    	}
    delete[] outbuf;
    return EXIT_SUCCESS;
    }
  	 	
int main_interleavedfastq(int argc,char** argv)
    {
    InterleavedFastq app;
    return app.doWork(argc,argv);
    }
