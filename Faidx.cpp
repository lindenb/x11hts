#include <cstring>
#include <cerrno>
#include "macros.hh"
#include "Faidx.hh"

using namespace std;

IndexedFastaSequence::IndexedFastaSequence(const char *fn):idx(0) {
    ASSERT_NOT_NULL(fn);
    idx = ::fai_load(fn);
    if(idx==NULL) FATAL("Cannot read fasta index from "<< fn << " "<< strerror(errno));
    }

IndexedFastaSequence::~IndexedFastaSequence() {
  ::fai_destroy(this->idx);
    }

shared_ptr<string> IndexedFastaSequence::fetch(const char *c_name, int p_beg_i, int p_end_i) {
    ASSERT_NOT_NULL(c_name);
    int len=0;
    char *p = ::faidx_fetch_seq(this->idx, c_name, p_beg_i, p_end_i,&len);
    if(p!=NULL)
	{
	string* s=new string(p);
	::free(p);
	return shared_ptr<string>(s);
	}
    else
	{
	return shared_ptr<string>();
	}
    }

int IndexedFastaSequence::size() {
    return ::faidx_nseq(this->idx);
}
const char* IndexedFastaSequence::getSequenceName(int i) {
    return ::faidx_iseq(this->idx, i);
    }

int IndexedFastaSequence::getSequenceLength(const char* seq) {
    return faidx_seq_len(this->idx, seq);
    }

bool IndexedFastaSequence::contains(const char* s) {
    return faidx_has_seq(idx,s)==1;
    }
