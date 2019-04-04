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
#ifndef HERSHEY_H
#define HERSHEY_H
#include <vector>
#include <cstdio>
#include <cstring>
#include <cstdlib>
#include <cctype>
#include <iostream>
#include <sstream>
#include "config.h"
#include "Graphics.hh"
#include "Utils.hh"



class Hershey
	{
	private:
		struct Operator
			{
			double x;
			double y;
			int op;
			};
		const int MOVETO;
		const int LINETO;
		std::vector<Operator> array;
	public:
		double scalex;
		double scaley;

		Hershey();
		virtual ~Hershey();
	private:
		const char* charToHersheyString(char c);
		void  charToPathOp(char letter);
	public:	
		void paint(
		    Graphics* g,
		    const char* s,
		    double x, double y,
		    double width, double height
		    );
		void svgPath(
			std::ostream& out,
			const char* s,
			double x, double y,
			double width, double height
			);
	};


#endif

