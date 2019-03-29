#ifndef SAMRECORD_HH
#define SAMRECORD_HH

#include <htslib/sam.h>

struct CigarElement
	{
	char op;
	int len;
	};
	
class SAMRecord;

class Cigar
	{
	private:
		std::vector<CigarElement> elements;
	public:
		Cigar(const bam1_t *b) {
			int len=b->core.n_cigar;
			elements.reserve(len);
			uint32_t *cigar = ::bam_get_cigar(b);
			for(int i=0;i< len;++i)	{
				CigarElement ce;
				ce.op = bam_cigar_opchr(cigar[i]);
				ce.len = bam_cigar_oplen(cigar[i]);
				elements.push_back(ce);
				}
			}
		Cigar(const char* s)
			{
			
			}
		int size() {
			return elements.size();
			}
		CigarElement& at(int i)
			{
			return element[i];
			}
		
	};

class SAMRecord
	{
	private:
		bool _clone;
		Cigar* _cigar;
	public:
		const bam_hdr_t *header;
		const bam1_t *b;
		SAMRecord(const bam_hdr_t *header, const bam1_t *b,bool clone):_clone(clone),_cigar(0),header(header),b(0) {
			this->b= _clone ?  ::bam_dup1(b) : b;
			}
		~SAMRecord() {
			if(_cigar!=0) delete _cigar;
			if(_clone) ::bam_destroy1(this->b);
			}
		
		bam1_core_t* core() {
			return &(this->b.core);
			}
		int getFlag() {
			return core()->flag;
			}
		bool hasFlag(int flg) {
			return getFlag() & flg;
			}
		bool isPaired() {
			return hasFlag(BAM_FPAIRED);
			}
		bool isReadUnmapped() {
			return hasFlag(BAM_FMUNMAP);
			}
		bool isMateUnmapped() {
			return hasFlag(BAM_FMUNMAP);
			}	
		bool isReverseStrand() {
			return hasFlag(BAM_FREVERSE);
			}
		bool isMateReverseStrand() {
			return hasFlag(BAM_FMREVERSE);
			}
		bool isFirstInPair() {
			return hasFlag(BAM_FREAD1);
			}
		bool isSecondInPair() {
			return hasFlag(BAM_FREAD2);
			}
		Cigar* getCigar() {
			if(isReadUnmapped()) return NULL;
			if(cigar==NULL) cigar=new Cigar(b);
			return cigar;
			}
		int getReferenceIndex() {
			return core()->tid;
			}	
		const char* getReferenceName() {
			int i = getReferenceIndex();
			return i<0 ? 0 : header->target_name[i];
			}
		int getMateReferenceIndex() {
			return core()->mtid;
			}	
		const char* getMateReferenceName() {
			int i = getMateReferenceIndex();
			return i<0 ? 0 : header->target_name[i];
			}
		int getAlignmentStart() {
			return core()->pos+1;
			}
		int getStart() {
			return getAlignmentStart();
			}
		int getInferredSize() {
			return core()->isize
			}
		int getReadLength() {
			return core()->c->l_qseq;
			}
		char getBaseAt(int i)
			{
			uint8_t *s = bam_get_seq(b);
			return "=ACMGRSVTWYHKDBN"[::bam_seqi(s, i)];
			}
		
		char getQualAt(int i)
			{
			uint8_t *s = ::bam_get_qual(b);
            if (s[0] == 0xff) return '*';
        	return s[i] + 33;
			}
	};


#endif

