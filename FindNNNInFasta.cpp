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
#include <unistd.h>
#include <getopt.h>
#include <zlib.h>

#include "macros.hh"
#include "Utils.hh"
#include "AbstractCmdLine.hh"
using namespace std;

class FindNNNInFasta : public AbstractCmd
    {
    public:
	bool stop_after_ws;
	FindNNNInFasta();
	virtual ~FindNNNInFasta();
	virtual void findNNN(gzFile in,FILE* out);
	virtual int doWork(int argc,char** argv);
    };
FindNNNInFasta::FindNNNInFasta():stop_after_ws(false) {
    app_desc.assign("find stretch of N in fasta. Output is a BED file.");
    app_name.assign("findnnninfasta");

    Option* opt = new Option('w',false,"name in fasta sequence stops after first whitespace");
    options.push_back(opt);
    }

FindNNNInFasta::~FindNNNInFasta() {
    }

#define DUMP if(prev<pos) fprintf(out,"%s\t%ld\t%ld\t%ld\n",chrom_name.c_str(),prev,pos,pos-prev)

void FindNNNInFasta::findNNN(gzFile in,FILE* out) {
    std::string chrom_name;
    long prev=0;
    long pos=0;
    int c;
    while((c=gzgetc(in))!=EOF) {
	if(c=='>') {
	    DUMP;
	    prev=0;
	    pos=0;
	    chrom_name.clear();
	    bool got_ws=false;
	    while((c=gzgetc(in))!=EOF && c!='\n') {
		if(this->stop_after_ws && (got_ws || isspace(c)))
		    {
		    got_ws = true;
		    }
		else
		    {
		    chrom_name+=(char)c;
		    }
		}
	    }
	else if(c=='N' || c=='n')
	    {
	    pos++;
	    }
	else if(!isspace(c)) {
	    DUMP;
	    pos++;
	    prev=pos;
	    }
	}
    DUMP;
    }

int FindNNNInFasta:: doWork(int argc,char** argv) {
       int opt;
       string optstr = this->build_getopt_str();
       while ((opt = ::getopt(argc, argv, optstr.c_str())) != -1) {
   	    switch (opt) {
	    case 'h':
		    usage(cout);
		    return 0;
	    case 'v':
		    cout << app_version << endl;
		    return 0;
	    case 'w':
		this->stop_after_ws = true;
		break;
   	    case '?':
   		    cerr << "unknown option -"<< (char)optopt << endl;
   		    return EXIT_FAILURE;
   	    default: /* '?' */
   		    cerr << "unknown option" << endl;
   		    return EXIT_FAILURE;
   	    }
   	}
    if(optind==argc)
	{
	gzFile in = ::gzdopen(fileno(stdin), "r");
	if(in==Z_NULL) {
	    FATAL("Cannot open stdin." << strerror(errno));
	    }
	findNNN(in,stdout);
	gzclose(in);
	}
    else while(optind<argc) {
	gzFile in = ::gzopen(argv[optind], "r");
	if(in==Z_NULL) {
	    FATAL("Cannot open "<< argv[optind] <<". " << strerror(errno));
	    }
	findNNN(in,stdout);
	gzclose(in);
	optind++;
	}
    return 0;
    }


int main_find_nnn_in_fasta(int argc,char** argv)
    {
    FindNNNInFasta app;
    return app.doWork(argc,argv);
    }
