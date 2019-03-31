/*
 * SAMFile.cpp
 *
 *  Created on: Mar 31, 2019
 *      Author: lindenb
 */

#include <sstream>
#include <cstring>
#include <cstdlib>
#include "SAMFile.hh"
#include "macros.hh"

using namespace std;

SAMFile::SAMFile():filename(NULL),fp(0),hdr(0),idx(0) {

}
SAMFile::~SAMFile() {
	if(filename!=NULL) free(filename);
	if(idx!=0) ::hts_idx_destroy(idx);
	if(hdr!=0) ::bam_hdr_destroy(hdr);
	if(fp!=0) ::hts_close(fp);
	}



int SAMFile::contigToTid(const char* chrom) {
	ASSERT_NOT_NULL(chrom);
	int tid = ::bam_name2id(this->hdr, chrom);
	bool chr_prefix =  strncmp(chrom,"chr",3)==0 ;
	if(tid<0 && chr_prefix )
		{
		tid = ::bam_name2id(this->hdr, &chrom[3]);
		}
	if(tid<0 && !chr_prefix)
		{
		string ctg2 = "chr";
		ctg2.append(chrom);
		tid = ::bam_name2id(this->hdr, ctg2.c_str());
		}
	return tid;
	}

SAMFileFactory::SAMFileFactory():require_index(true) {
}

SAMFileFactory::~SAMFileFactory()
	{
	}

SAMFile* SAMFileFactory::createInstance() {
	return new SAMFile;
}

SAMFile* SAMFileFactory::open(const char* fn) {
	ASSERT_NOT_NULL(fn);
	SAMFile* instance = createInstance();
	ASSERT_NOT_NULL(instance);
	instance->filename = ::strdup(fn);
	ASSERT_NOT_NULL(instance->filename);
	instance->fp = ::hts_open(fn, "r");
	if(instance->fp==NULL) {
			delete instance;
			FATAL( "Cannot open " << fn << ". " << ::strerror(errno));
			}
	instance->hdr = ::sam_hdr_read(instance->fp);
	if (instance->hdr == NULL) {
		delete instance;
		FATAL("Cannot open header for " << fn << ".");
		}

	if(this->require_index) {
		instance->idx = ::sam_index_load(instance->fp,fn);
		if (instance->idx==NULL) {
			delete instance;
			FATAL("Cannot open index for " << fn << "." );
			}
		}

	if(instance->hdr->text!=NULL)
		{
		std::istringstream iss(instance->hdr->text);
		std::string line;
	    while(std::getline(iss, line, '\n'))
			{
	    	string::size_type  p = line.find("@RG\t");
	    	if(p!=0) continue;
	    	p = line.find("\tSM:");
			if(p==string::npos) continue;
			p+=4;
			string::size_type  p2 = line.find("\t",p);
			if(p2==string::npos) p2=line.size();
			instance->samples.insert( line.substr(p,(p2-p)));
			break;
			}
		}
	return instance;
	}



