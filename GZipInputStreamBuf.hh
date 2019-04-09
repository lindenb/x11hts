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
#ifndef GZIPINPUTSTREAMBUF_HH
#define GZIPINPUTSTREAMBUF_HH


#include <iostream>
#include <vector>
#include <string>
#include <zlib.h>
#include <cstdlib>
#include <cstdio>
#include <cstring>
#include <unistd.h>
#include <getopt.h>
#include <cassert>


class GzipInputStreamBuf : public std::basic_streambuf<char>
	{
	private:
		std::size_t buffer_size;
		char* buffer;
		gzFile gzin;
		void init_buffer(std::size_t n);
	public:
		GzipInputStreamBuf(const char* fname);
		GzipInputStreamBuf(int);
		GzipInputStreamBuf(const char* fname,std::size_t buf_size);
		GzipInputStreamBuf(int,std::size_t buf_size);
		virtual ~GzipInputStreamBuf();
        protected:
		virtual int underflow( );
	};

#endif

