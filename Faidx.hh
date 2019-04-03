#ifndef FAIDX_HH
#define FAIDX_HH
#include <memory>
#include <htslib/faidx.h>


class IndexedFastaSequence
    {
    private:
	faidx_t *idx;
    public:
	IndexedFastaSequence(const char* fn);
	~IndexedFastaSequence();
	int size();
	const char* getSequenceName(int i);
	int getSequenceLength(const char* s);
	bool contains(const char* s);
	std::shared_ptr<std::string> fetch(const char *c_name, int p_beg_i, int p_end_i);
    };


#endif
