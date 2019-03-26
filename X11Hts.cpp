/*
 * X11Bam.cpp
 *
 *  Created on: Mar 25, 2019
 *      Author: lindenb
 */
#include <cstring>
#include <cstdlib>
#include <iostream>

using namespace std;

extern int main_cnv(int argc,char** argv);

static void usage() {

}

int main(int argc,char** argv) {
	if(argc<2) {
		usage();
		return EXIT_FAILURE;
		}
	if(strcmp(argv[1],"cnv")==0) {
		return main_cnv(argc,&argv[1]);
		}
	else
		{
		cerr << "unknown command \""<< argv[1] << "\"." << endl;
		return EXIT_FAILURE;
		}
	}
