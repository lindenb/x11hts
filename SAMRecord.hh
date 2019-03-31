#ifndef SAMRECORD_HH
#define SAMRECORD_HH

#include <vector>
#include <htslib/sam.h>
#include <htslib/faidx.h>

struct CigarOperator {
	char op;
	bool consumeRead;
	bool consumeRef;
	CigarOperator(char op,bool consumeRead,bool consumeRef);
	bool isMatch() const;
	bool isDeletion() const;
	bool isClipping() const;

	static const CigarOperator* valueOf(char c);
	static const CigarOperator* M;
	static const CigarOperator* X;
	static const CigarOperator* EQ;
	static const CigarOperator* N;
	static const CigarOperator* D;
	static const CigarOperator* I;
	static const CigarOperator* P;
	static const CigarOperator* S;
	static const CigarOperator* H;
};


struct CigarElement
	{
	const CigarOperator* op;
	int len;
	CigarElement(const CigarOperator* op,int len);
	};
	
class SAMRecord;

class Cigar
	{
	private:
		std::vector<CigarElement> elements;
	public:
		Cigar(const bam1_t *b);
		int size();
		CigarElement& at(int i);
	};

class SAMRecord
	{
	private:
		bool _clone;
		Cigar* _cigar;
	public:
		const bam_hdr_t *header;
		bam1_t *b;
		SAMRecord(const bam_hdr_t *header, bam1_t *b,bool clone);
		~SAMRecord();
		bam1_core_t* cor();
		const char* getReadName();
		int getFlag();
		bool hasFlag(int flg);
		bool isPaired();
		bool isReadUnmapped();
		bool isMateUnmapped();
		bool isReverseStrand();
		bool isMateReverseStrand();
		bool isFirstInPair();
		bool isSecondInPair();
		Cigar* getCigar();
		int getReferenceIndex();
		const char* getReferenceName();
		int getMateReferenceIndex();
		const char* getMateReferenceName();
		int getAlignmentStart();
		int getAlignmentEnd();
		int getStart();
		int getInferredSize();
		int getReadLength();
		char getBaseAt(int i);
		char getQualAt(int i);
	};


#endif

