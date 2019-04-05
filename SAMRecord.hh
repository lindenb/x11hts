#ifndef SAMRECORD_HH
#define SAMRECORD_HH

#include "Locatable.hh"
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
		int getReferenceLength() const;
		int getRightClipLength() const;
		int getLeftClipLength() const;
	};

class SAMRecord: public Locatable
	{
	private:
		bool _clone;
		Cigar* _cigar;
		int mAlignmentEnd;
	public:
		const bam_hdr_t *header;
		bam1_t *b;
		SAMRecord(const bam_hdr_t *header, bam1_t *b,bool clone);
		~SAMRecord();
		bam1_core_t* cor();
		const char* getReadName();
		int getReadNameLength();
		int getFlag() const;
		bool hasFlag(int flg) const;
		bool isPaired() const ;
		bool isReadUnmapped() const;
		bool isMateUnmapped() const;
		bool isReverseStrand() const ;
		bool isMateReverseStrand() const;
		bool isFirstInPair() const;
		bool isSecondInPair() const;
		bool isProperPair() const;
		bool isSecondaryAlignment() const;
		bool isSupplementaryAlignment() const;
		bool isSecondaryOrSupplementaryAlignment() const;
		Cigar* getCigar();
		int getReferenceIndex();
		const char* getReferenceName();
		int getMateReferenceIndex();
		const char* getMateReferenceName();
		int getAlignmentStart();
		int getAlignmentEnd();
		int getUnclippedStart();
		int getUnclippedEnd();
		const char* getContig() const;
		int getStart() const;
		int getEnd() const;
		int getInferredSize();
		int getReadLength();
		char getBaseAt(int i);
		char getQualAt(int i);
	};


#endif

