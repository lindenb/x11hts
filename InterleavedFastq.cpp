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
#include "KString.hh"
#include "Utils.hh"

using namespace std;


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
    }

InterleavedFastq::~InterleavedFastq() {

}
#define MUST_READ(IN,L) \
	if(!KString::readLine(IN, L)) FATAL("line missing in fastq " << L1 << ".")

#define WRITE_LINE(L) L.write(stdout);fputc('\n',stdout)

int InterleavedFastq::doWork(int argc,char** argv) {
    int opt;
    int modulo=-1;
    int divider =0;
    string optstr = this->build_getopt_str();
    while ((opt = ::getopt(argc, argv, optstr.c_str())) != -1) {
	    switch (opt) {
	    case 'h':
		    usage(cout);
		    return 0;
	    case 'm':
		    modulo = Utils::parseInt(optarg);
		    break;
		case 'd':
		    divider = Utils::parseInt(optarg);
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
    KString L1;
    KString L2;
    KString L3;
    KString L4;


    long nReads  = 0;
    if(optind==argc || optind+1==argc) {
	    //single end
	    gzFile in=
		    (optind==argc?
		      ::gzdopen(fileno(stdin),"r"):
		      ::gzopen(argv[optind],"r")
		     );
	    for(;;) {
		if(!KString::readLine(in, L1)) break;
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
	   gzclose(in);
	   cout.flush();
	  }
	else if(optind+2==argc) {
	    //paired end
	    gzFile in1 = ::gzopen(argv[optind  ],"r");
	    gzFile in2 = ::gzopen(argv[optind+1],"r");
	    KString L5;
	    KString L6;
	    KString L7;
	    KString L8;

	    for(;;) {
		if(!KString::readLine(in1, L1)) {
		    if(KString::readLine(in2, L5)) {
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
	    gzclose(in1);
	    gzclose(in2);
	    cout.flush();
	  }
    else {
    	cerr << "Illegal Number of arguments." << endl;
    	return EXIT_FAILURE;
    	}

    return EXIT_SUCCESS;
    }
  	 	
int main_interleavedfastq(int argc,char** argv)
    {
    InterleavedFastq app;
    return app.doWork(argc,argv);
    }
