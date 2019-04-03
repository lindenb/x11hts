
#ifndef BEDLINE_HH
#define BEDLINE_HH
#include <string>
#include <iostream>
#include <vector>


class BedCodec
{
public:
	BedCodec();
	virtual ~BedCodec();
	virtual bool is_ignore(const std::string& line);
	virtual bool parse(const std::string& line,std::string* chrom,int* start0,int* end0);
	virtual bool parse(const std::vector<std::string>& tokens,std::string* chrom,int* start0,int* end0);
};

#endif
