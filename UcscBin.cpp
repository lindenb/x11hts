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
#include "UcscBin.hh"

using namespace std;


UcscBin::bin_t UcscBin::reg2bin(std::uint32_t beg, std::uint32_t end) {
    --end;
    if (beg>>14 == end>>14) return 4681 + (beg>>14);
    if (beg>>17 == end>>17) return  585 + (beg>>17);
    if (beg>>20 == end>>20) return   73 + (beg>>20);
    if (beg>>23 == end>>23) return    9 + (beg>>23);
    if (beg>>26 == end>>26) return    1 + (beg>>26);
    return 0;
}
UcscBin::bin_t UcscBin::reg2bin(const Locatable* loc) {
    return  UcscBin::reg2bin(loc->getStart()-1,loc->getEnd());
}


#define MAX_BIN 37450 // =(8^6-1)/7+1

std::size_t UcscBin::reg2bins(uint32_t beg, uint32_t end, std::vector<UcscBin::bin_t>& list) {
    int i = 0, k;
    list.clear();
    list.reserve(MAX_BIN);
    if (beg >= end) return 0;
    if (end >= 1u<<29) end = 1u<<29;
    --end;
    list.push_back(0);
    for (k =    1 + (beg>>26); k <=    1 + (end>>26); ++k) list.push_back(k);
    for (k =    9 + (beg>>23); k <=    9 + (end>>23); ++k) list.push_back(k);
    for (k =   73 + (beg>>20); k <=   73 + (end>>20); ++k) list.push_back(k);
    for (k =  585 + (beg>>17); k <=  585 + (end>>17); ++k) list.push_back(k);
    for (k = 4681 + (beg>>14); k <= 4681 + (end>>14); ++k) list.push_back(k);
    return list.size();
    }
