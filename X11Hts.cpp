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
#include <cstring>
#include <cstdlib>
#include <iostream>
#include "AbstractCmdLine.hh"

using namespace std;

class X11Hts : public AbstractCmd
    {
    public:
	X11Hts();
	virtual ~X11Hts();
	virtual void usage(std::ostream& out);
	virtual int doWork(int argc,char** argv);
    };

extern int main_cnv(int argc,char** argv);
//extern int main_browse(int argc,char** argv);
extern int main_splitfastq(int argc,char** argv);
extern int main_interleavedfastq(int argc,char** argv);

X11Hts::X11Hts() {
    app_name="x11hts";
}
X11Hts::~X11Hts() {
}

void X11Hts::usage(std::ostream& out) {
AbstractCmd::usage(out);

out << "Usage:" << endl;
out << "    "<< app_name << " cnv [options]" << endl;
out << "    "<< app_name << " browse [options]" << endl;
out << "    "<< app_name << " splitfastq [options]" << endl;
out << "    "<< app_name << " interleavedfastq [options]" << endl;

out << endl;
}

int X11Hts::doWork(int argc,char** argv) {
	if(argc<2) {
		usage(cerr);
		return EXIT_FAILURE;
		}
	try	{
		if(strcmp(argv[1],"cnv")==0) {
			return main_cnv(argc-1,&argv[1]);
			}
		//else if(strcmp(argv[1],"browse")==0) { return main_browse(argc-1,&argv[1]);}
		else if(strcmp(argv[1],"splitfastq")==0) { return main_splitfastq(argc-1,&argv[1]);}
		else if(strcmp(argv[1],"interleavedfastq")==0) { return main_interleavedfastq(argc-1,&argv[1]);}
		else
			{
			cerr << "unknown command \""<< argv[1] << "\"." << endl;
			return EXIT_FAILURE;
			}
		}
	catch(std::exception& err)
		{
		cerr << "[FATAL] An error occured \""<< err.what() << "\"." << endl;
		return EXIT_FAILURE;
		}
	catch(...)
		{
		cerr << "[FATAL] An error occured." << endl;
		return EXIT_FAILURE;
		}
	}

int main(int argc,char** argv) {
    X11Hts app;
    return app.doWork(argc, argv);
}
