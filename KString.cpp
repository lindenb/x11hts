#include <cstring>
#include "KString.hh"

using namespace std;



KString::KString(std::size_t capacity) {
    this->l=0;
    this->m=0;
    this->s = 0;
    ks_resize(this,capacity<1?10:capacity);
    this->s[0]=0;
    }

KString::KString(const KString& cp) {
    this->l=0;
    this->m=0;
    this->s = 0;
    ks_resize(this,cp.size()+1);
    strncpy(this->s,cp.s,cp.size());
    this->l=cp.size();
    this->s[this->l]=0;
    }


KString::KString() {
    this->l=0;
    this->m=0;
    this->s = 0;
    ks_resize(this,10);
    this->s[0]=0;
    }



KString::~KString() {
free(this->s);
}

const char* KString::c_str() const {
    return this->s;
}
const std::size_t KString::size() const {
    return this->l;
}
const std::size_t KString::length() const {
    return size();
}
char KString::at(std::size_t i) const {
    return this->s[i];
}
char& KString::at(std::size_t i) {
    return this->s[i];
}

void KString::clear() {
    this->l = 0;
    this->s[0]=0;
}

bool KString::empty() const {
    return size()==0UL;
}
KString& KString::append(char c) {
    kputc(c,this);
    return *this;
}
KString& KString::append(const char* s) {
    return append(s,strlen(s));
}
KString& KString::append(const char* s,std::size_t len) {
    kputsn(s,len,this);
    return *this;
}

std::string KString::str() const {
    return string(c_str(),size());
}

void KString::write(std::FILE* out) {
    ::fwrite((void*)this->s, sizeof(char), this->l,out);
}

bool KString::readLine(gzFile in,KString& line) {
    line.clear();
    if(gzeof(in)) return false;
    int c;
    bool got_one=false;
    while((c=gzgetc(in))!=EOF ) {
	got_one=true;
	if(c=='\n') break;
	kputc(c,&line);
	}

    if(!got_one) {
	return false;
	}

    if (line.l > 0 && line.s[line.l - 1] == '\r') {
    		line.l--;
    	    }
    line.s[line.l] = 0;
    return true;
}

std::ostream& operator<<(std::ostream& os, const KString& k) {
    os.write(k.s, k.l);
    return os;
    }
