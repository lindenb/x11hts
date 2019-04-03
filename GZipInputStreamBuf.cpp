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
#include <cerrno>
#include <cstring>
#include "macros.hh"
#include "GZipInputStreamBuf.hh"

using namespace std;

#define BUFFER_SIZE 4096


void GzipInputStreamBuf::init_buffer(std::size_t n) {
this->buffer_size= n;
this->buffer=new char[this->buffer_size];
this->setg(
    (char*)&this->buffer[0],
    (char*)&this->buffer[this->buffer_size],
    (char*)&this->buffer[this->buffer_size]
    );
}

GzipInputStreamBuf::GzipInputStreamBuf(const char* fname):buffer_size(0UL),buffer(0),gzin(0)
	{
	ASSERT_NOT_NULL(fname);
	this->gzin = ::gzopen(fname,"r");
	if(this->gzin == Z_NULL)
	    {
	    FATAL("Cannot open file: " << fname <<  " "<< strerror(errno));
	    }
	init_buffer(BUFFER_SIZE);
	}

GzipInputStreamBuf::GzipInputStreamBuf(int fd):buffer_size(BUFFER_SIZE),buffer(0),gzin(0)
	{
	this->gzin = ::gzdopen(fd, "r");
	if(this->gzin == Z_NULL)
	    {
	    FATAL("Cannot open file descriptor: " << fd <<  " "<< strerror(errno));
	    }
	init_buffer(BUFFER_SIZE);
	}

GzipInputStreamBuf::~GzipInputStreamBuf()
	{
	if(this->gzin!=Z_NULL) ::gzclose(this->gzin);
	if(this->buffer!=0) delete [] this->buffer;
	}

int GzipInputStreamBuf::underflow ( )
	{
	int nRead =0;
	if(gzeof(this->gzin)) return EOF;

	if( ( nRead = ::gzread(this->gzin,this->buffer,this->buffer_size) ) <= 0 ) {
		int ret = 0;
		const char* msg = ::gzerror(this->gzin,&ret);
		if(ret!=0) FATAL("gz error " << msg);
		return EOF;
		}

	setg(	(char*)&this->buffer[0],
		(char*)&this->buffer[0],
		(char*)&this->buffer[nRead]
		);

	return this->buffer[0];
	}
