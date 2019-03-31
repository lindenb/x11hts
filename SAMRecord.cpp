#include "SAMRecord.hh"
#include "macros.hh"

using namespace std;
CigarOperator::CigarOperator(char op,bool consumeRead,bool consumeRef):op(op),consumeRead(consumeRead),consumeRef(consumeRef) {
}

bool CigarOperator::isMatch() const {
	switch(this->op) {
	case 'M':case 'X': case '=': return true;
	default: return false;
	}
}

bool CigarOperator::isDeletion() const {
	switch(this->op) {
	case 'D':case 'N':return true;
	default: return false;
	}
}

bool CigarOperator::isClipping() const {
	switch(this->op) {
		case 'H':case 'S': return true;
		default: return false;
		}
}



const CigarOperator* CigarOperator::P = new CigarOperator('P',false,false);
const CigarOperator* CigarOperator::M = new CigarOperator('M',true,true);
const CigarOperator* CigarOperator::EQ = new CigarOperator('=',true,true);
const CigarOperator* CigarOperator::X = new CigarOperator('X',true,true);
const CigarOperator* CigarOperator::D = new CigarOperator('D',false,true);
const CigarOperator* CigarOperator::I = new CigarOperator('I',true,false);
const CigarOperator* CigarOperator::N = new CigarOperator('N',false,true);
const CigarOperator* CigarOperator::H = new CigarOperator('H',false,false);
const CigarOperator* CigarOperator::S = new CigarOperator('S',false,false);

const CigarOperator* CigarOperator::valueOf(char c) {
	switch(c) {
		case 'P': return CigarOperator::P;
		case '=': return CigarOperator::EQ;
		case 'X': return CigarOperator::X;
		case 'M': return CigarOperator::M;
		case 'N': return CigarOperator::N;
		case 'D': return CigarOperator::D;
		case 'I': return CigarOperator::I;
		case 'H': return CigarOperator::H;
		case 'S': return CigarOperator::S;
		default:FATAL("unknown cigar operator "<< c);return NULL;
		}
	}

CigarElement::CigarElement(const CigarOperator* op,int len):op(op),len(len) {
	ASSERT_NOT_NULL(op);
	assert(len>0);
	}

Cigar::Cigar(const bam1_t *b) {
	ASSERT_NOT_NULL(b);
	int len=b->core.n_cigar;
	assert(len>=0);
	this->elements.reserve(len);
	uint32_t *cigar = bam_get_cigar(b);
	for(int i=0;i< len;++i)	{
		CigarElement ce(
			CigarOperator::valueOf(bam_cigar_opchr(cigar[i])),
			bam_cigar_oplen(cigar[i])
			);
		this->elements.push_back(ce);
		}
	}

int Cigar::size() {
	return elements.size();
	}
CigarElement& Cigar::at(int i)
	{
	return this->elements[i];
	}



SAMRecord::SAMRecord(const bam_hdr_t *header, bam1_t *b,bool clone):_clone(clone),_cigar(0),header(header),b(0) {
	this->b= _clone ?  ::bam_dup1(b) : b;
	}

SAMRecord::~SAMRecord() {
	if(_cigar!=0) delete _cigar;
	if(_clone) ::bam_destroy1(this->b);
	}

bam1_core_t* SAMRecord::cor() {
	return &(this->b->core);
	}

int SAMRecord::getFlag() {
	return cor()->flag;
	}
bool SAMRecord::hasFlag(int flg) {
	return getFlag() & flg;
	}
bool SAMRecord::isPaired() {
	return hasFlag(BAM_FPAIRED);
	}
bool SAMRecord::isReadUnmapped() {
	return hasFlag(BAM_FMUNMAP);
	}
bool SAMRecord::isMateUnmapped() {
	return hasFlag(BAM_FMUNMAP);
	}
bool SAMRecord::isReverseStrand() {
	return hasFlag(BAM_FREVERSE);
	}
bool SAMRecord::isMateReverseStrand() {
	return hasFlag(BAM_FMREVERSE);
	}
bool SAMRecord::isFirstInPair() {
	return hasFlag(BAM_FREAD1);
	}
bool SAMRecord::isSecondInPair() {
	return hasFlag(BAM_FREAD2);
	}
Cigar* SAMRecord::getCigar() {
	if(isReadUnmapped()) return NULL;
	if(_cigar==NULL) _cigar=new Cigar(b);
	return _cigar;
	}
int SAMRecord::getReferenceIndex() {
	return cor()->tid;
	}
const char* SAMRecord::getReferenceName() {
	int i = getReferenceIndex();
	return i<0 ? 0 : header->target_name[i];
	}
int SAMRecord::getMateReferenceIndex() {
	return cor()->mtid;
	}
const char* SAMRecord::getMateReferenceName() {
	int i = getMateReferenceIndex();
	return i<0 ? 0 : header->target_name[i];
	}
int SAMRecord::getAlignmentStart() {
	return cor()->pos+1;
	}
int SAMRecord::getStart() {
	return getAlignmentStart();
	}
int SAMRecord::getInferredSize() {
	return cor()->isize;
	}
int SAMRecord::getReadLength() {
	return cor()->l_qseq;
	}
char SAMRecord::getBaseAt(int i)
	{
	uint8_t *s = bam_get_seq(b);
	return "=ACMGRSVTWYHKDBN"[bam_seqi(s, i)];
	}

char SAMRecord::getQualAt(int i)
	{
	uint8_t *s = bam_get_qual(b);
	if (s[0] == 0xff) return '*';
	return s[i] + 33;
	}

