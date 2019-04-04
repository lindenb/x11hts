#ifndef X11SAMFILE_HH
#define X11SAMFILE_HH

#include <set>
#include <vector>
#include <map>
#include <string>
#include <htslib/sam.h>
#include <htslib/faidx.h>
#include <htslib/kstring.h>
#include <htslib/khash_str2int.h>

class SAMReadGroupRecord
    {
    public:
	std::string id;
	std::string sample;
    };

class SAMFile
	{
	public:
		char* filename;
		std::map<std::string,SAMReadGroupRecord> rgid2rg;
		samFile *fp;
		bam_hdr_t *hdr;  // the file header
		hts_idx_t *idx;

		SAMFile();
		virtual ~SAMFile();
		/** returns the first sample in this BAM, or the empty string */
		virtual std::string getSample() const;
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




