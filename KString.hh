#ifndef KSTRING_HH
#define KSTRING_HH

#include <cstdio>
#include <iostream>
#include <string>
#include <zlib.h>
#include <htslib/kstring.h>


class KString : public kstring_t {
    public:
	KString(std::size_t capacity);
	KString();
	KString(const KString& cp);
	virtual ~KString();
	virtual const char* c_str() const;
	virtual const std::size_t size() const;
	virtual const std::size_t length() const;
	virtual char at(std::size_t i) const;
	virtual char& at(std::size_t i);
	virtual void clear();
	virtual bool empty() const;
	virtual KString& append(char c);
	virtual KString& append(const char* s);
	virtual KString& append(const char* s,std::size_t len);
	virtual std::string str() const;
	virtual void write(std::FILE* out);


	static bool readLine(gzFile in,KString& line);
    };

std::ostream& operator<<(std::ostream& os, const KString& k);

#endif
