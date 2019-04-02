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
#include "GZipInputStreamBuf.hh"

using namespace std;

#define PATTERN "__SPLIT__"

class Chunck
	{
	public:
		string filename;
		gzFile out;
		unsigned long count;
	};

class SplitFastq
    {
    public:
		void usage(std::ostream& out);
		int doWork(int argc,char** argv);
    };

void SplitFastq::usage(std::ostream& out) {
    out << "-n number of splits" << endl;
    out << "-o (fileout) must end with .gz and contains " << PATTERN << endl;
}

int SplitFastq::doWork(int argc,char** argv) {
    int opt;
    int force=0;
    char* file_out = NULL;
    int nsplits=0;
    while ((opt = getopt(argc, argv, "hn:fo:")) != -1) {
	    switch (opt) {
	    case 'h':
		    usage(cout);
		    return 0;
	    case 'n':
		    nsplits =atoi(optarg);
		    break;
	    case 'o':
		    file_out = optarg;
		    break;
	    case 'f':
		    force = 1;
		    break;
	    case '?':
		    cerr << "unknown option -"<< (char)optopt << endl;
		    return EXIT_FAILURE;
	    default: /* '?' */
		    cerr << "unknown option" << endl;
		    return EXIT_FAILURE;
	    }
	}
    if(nsplits<=0)  {
	cerr << "Bad value for nsplit :" << nsplits << endl;
	return EXIT_FAILURE;
    }
    if(file_out==NULL)  {
   	cerr << "No output defined."<< endl;
   	return EXIT_FAILURE;
       }
    if(strstr(file_out,PATTERN)==NULL)  {
      	cerr << "output file " << file_out << " must contain " << PATTERN << endl;
      	return EXIT_FAILURE;
        }
    if(strlen(file_out)<3 || strcmp(&file_out[strlen(file_out)-3],".gz")!=0)  {
	cerr << "output file " << file_out << " must end with  .gz" << endl;
	return EXIT_FAILURE;
        }

    if(optind+1!=argc) {
	    cerr << "Illegal number of arguments." << endl;
	    return EXIT_FAILURE;
	    }
   GzipInputStreamBuf gzbuf(argv[optind]);


   vector<Chunck*> chunks;
   for(int i=0;i< nsplits;i++)
       {
       Chunck* chunk = new Chunck;
       chunk->filename.assign(file_out);
       chunk->out = NULL;
       chunk->count = 0UL;
       
       string::size_type p;
       char tmp[10];
       snprintf(tmp,10,"%04d",(i+1));
       while((p=chunk->filename.find(PATTERN))!=string::npos) {
	    chunk->filename.replace(p,strlen(PATTERN),tmp);
	   	}
       chunks.push_back(chunk);
       

       if(force==0) {
		   FILE* f=fopen(chunk->filename.c_str(),"rb");
		   if(f!=0) {
			   fclose(f);
			   FATAL("File '" << chunk->filename << "' already exists. Use -f to force.");
			   }
		   }
       }
   for(size_t i=0;i< chunks.size();i++) {
       chunks[i]->out = gzopen(chunks[i]->filename.c_str(),"wb9");
       if(chunks[i]->out==Z_NULL) {
            cerr << "Cannot write " << chunks[i]->filename << " " << strerror(errno) << endl;
            return EXIT_FAILURE;
            }
       }
   int nline=0;
   int file_idx = 0;
   string line;
   istream in(&gzbuf);
   Chunck* chunk = chunks[file_idx];
   while(getline(in,line,'\n'))
       {
       gzwrite(chunk->out,(void*)line.c_str(), line.size());
       gzputc(chunk->out,'\n');
       nline++;
       if(nline==4) {
		   chunk->count++;
		   nline=0;
		   file_idx++;
		   if(file_idx==nsplits) file_idx=0;
		   chunk=chunks[file_idx];
	   	   }
       }
   if(nline!=0) {
       cerr << "Illegal number of reads  in input !" << endl;
       return EXIT_FAILURE;
   }

   for(size_t i=0;i< chunks.size();i++) {
       gzflush(chunks[i]->out,Z_FULL_FLUSH);
       gzclose(chunks[i]->out);
       cout << chunks[i]->filename << "\t" << chunks[i]->count << endl;
       }
   return 0;
   }

int main(int argc,char** argv)
    {
    SplitFastq app;
    return app.doWork(argc,argv);
    }
