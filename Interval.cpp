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
#include <cerrno>
#include <cstdlib>
#include "Interval.hh"
#include "Utils.hh"

using namespace std;

Interval::Interval(const char* ctg,int start,int end):
	_chrom(ctg),
	_start(start),
	_end(end) {
    }

Interval::~Interval() {
    }

const char* Interval::getContig() const {
    return _chrom.c_str();
    }
int Interval::getStart() const {
    return this->_start;
    }
int Interval::getEnd() const {
    return this->_end;
    }

std::shared_ptr<Interval> Interval::parse(const char* s) {
    shared_ptr<Interval> ret;
    if(Utils::isBlank(s)) return ret;
    const char* colon_ptr = std::strchr(s,':');
    if(colon_ptr==NULL || colon_ptr==s) return ret;
    char* hyphen_or_plus_ptr =  (char*)std::strchr(colon_ptr+1,'-');
    if(hyphen_or_plus_ptr==NULL)  hyphen_or_plus_ptr =  (char*)std::strchr(colon_ptr+1,'+');
    if(hyphen_or_plus_ptr==NULL) return ret;

    string ctg(s,0,colon_ptr-s);
    string s1(colon_ptr+1,hyphen_or_plus_ptr-colon_ptr);
    string s2(hyphen_or_plus_ptr+1);

    char* p2;
    int num1 = strtol(s1.c_str(),&p2,10);
    if(num1<0 || errno!=0 || *p2!=0) return ret;

    int num2 = strtol(s2.c_str(),&p2,10);
    if(num2<0 || errno!=0 || *p2!=0 ) return ret;


    if(*hyphen_or_plus_ptr=='+')
	{
	int L=std::max(1,num1-num2);
	int R=num1+num2;
	num1 =L;
	num2= R;
	}
    if(num1<1) num1=1;
    if(num2<num1) return ret;
    ret.reset(new Interval(ctg.c_str(),num1,num2));
    return ret;
    }


ostream& operator<<(std::ostream& os, const Interval& R) {
    os << R.getContig() << ":" << R.getStart() << "-" << R.getEnd();
    return os;
    }

