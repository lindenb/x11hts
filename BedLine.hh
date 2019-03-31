
#ifndef BEDLINE_HH
#define BEDLINE_HH
#include <string>
#include <iostream>
#include <vector>

class Interval
	{
	public:
		std::string contig;
		int start;
		int end;
		int length() const;
	};

class BedCodec
{
public:
	BedCodec();
	virtual ~BedCodec();
	virtual Interval* createInstance();
	virtual Interval* parse(const std::vector<std::string>& v);
	virtual Interval* parse(const std::string& line);
	virtual Interval* next(std::istream& in);
	virtual bool is_ignore(const std::string line);
	virtual size_t parseBedFile(const char* fn,std::vector<Interval*>& all);

};

#endif
