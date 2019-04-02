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
#include <cerrno>
#include <cstring>
#include <unistd.h>
#include <getopt.h>
#include <cassert>

using namespace std;

#define PATTERN "__SPLIT__"

#define BUFFER_SIZE 4096


class GzipStreamBuf : public std::basic_streambuf<char>
	{
	private:
		char* buffer;
		gzFile gzin;
	public:
		GzipStreamBuf(const char* fname)
			{
			this->gzin = ::gzopen(fname,"r");
			if(this->gzin == Z_NULL)
				{
				cerr << "Cannot open file: " << fname <<  " "<< strerror(errno) << std::endl;
				exit(EXIT_FAILURE);
				}

			this->buffer=new char[BUFFER_SIZE];

			setg(	(char*)&this->buffer[0],
				(char*)&this->buffer[BUFFER_SIZE],
				(char*)&this->buffer[BUFFER_SIZE]
				);
			}

		virtual ~GzipStreamBuf()
			{
			if(gzin!=NULL) ::gzclose(this->gzin);
			if(this->buffer!=NULL) delete [] this->buffer;
			}

		virtual int underflow ( )
			{
			int nRead =0;
			if(gzeof(this->gzin)) return EOF;

			if( ( nRead = ::gzread(this->gzin,this->buffer,BUFFER_SIZE) ) <= 0 ) {
				int ret = 0;
				const char* msg = ::gzerror(this->gzin,&ret);
				if(ret!=0) {
				    cerr << "gz error " << msg << endl;
				    exit(EXIT_FAILURE);
				    }
				return EOF;
				}

			setg(	(char*)&this->buffer[0],
				(char*)&this->buffer[0],
				(char*)&this->buffer[nRead]
				);

			return this->buffer[0];
			}
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
   GzipStreamBuf gzbuf(argv[optind]);


   vector<gzFile> gzouts;
   vector<unsigned long> counts;
   vector<string> filenames;
   for(int i=0;i< nsplits;i++)
       {
       string filename(file_out);
       string::size_type p;
       char tmp[10];
       snprintf(tmp,10,"%04d",(i+1));
       while((p=filename.find(PATTERN))!=string::npos) {
	    filename.replace(p,strlen(PATTERN),tmp);
	   }
       counts.push_back(0UL);
       filenames.push_back(filename);
       if(force==0) {
	   FILE* f=fopen(filename.c_str(),"rb");
	   if(f!=0) {
	       fclose(f);
	       cerr << "file " << filename << " already exists. Use -f to force" << endl;
	       return EXIT_FAILURE;
	       }
	   }
       }
   for(size_t i=0;i< filenames.size();i++) {
       gzFile gzout = gzopen(filenames[i].c_str(),"wb9");
       if(gzout==Z_NULL) {
            cerr << "Cannot write " << filenames[i] << " " << strerror(errno) << endl;
            return EXIT_FAILURE;
            }
       gzouts.push_back(gzout);
       }
   int nline=0;
   int file_idx = 0;
   string line;
   istream in(&gzbuf);
   gzFile gzout=gzouts[file_idx];
   while(getline(in,line,'\n'))
       {
       gzwrite(gzout,(void*)line.c_str(), line.size());
       gzputc(gzout,'\n');
       nline++;
       if(nline==4) {
	   counts[file_idx]++;
	   nline=0;
	   file_idx++;
	   if(file_idx==nsplits) file_idx=0;
	   gzout=gzouts[file_idx];
	   }
       }
   if(nline!=0) {
       cerr << "Illegal number of reads  in input !" << endl;
       return EXIT_FAILURE;
   }

   for(size_t i=0;i< gzouts.size();i++) {
       gzflush(gzouts[i],Z_FULL_FLUSH);
       gzclose(gzouts[i]);
       cout << filenames[i] << "\t" << counts[i] << endl;
       }
   return 0;
   }

int main(int argc,char** argv)
    {
    SplitFastq app;
    return app.doWork(argc,argv);
    }
