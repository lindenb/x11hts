#ifndef X11SAMFILE_HH
#define X11SAMFILE_HH

#include <set>
#include <string>
#include <htslib/sam.h>
#include <htslib/faidx.h>
#include <htslib/kstring.h>
#include <htslib/khash_str2int.h>


class SAMFile
	{
	public:
		char* filename;
		std::set<std::string> samples;
		samFile *fp;
		bam_hdr_t *hdr;  // the file header
		hts_idx_t *idx;

		SAMFile();
		virtual ~SAMFile();
		int contigToTid(const char* contig);
	};

class SAMFileFactory
{
public:
	bool require_index;
	SAMFileFactory();
	virtual ~SAMFileFactory();
	virtual SAMFile* createInstance();
	virtual SAMFile* open(const char* fn);
};


#endif




