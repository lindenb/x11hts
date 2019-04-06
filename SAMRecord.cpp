
#include <cstring>
#include "SAMRecord.hh"
#include "macros.hh"

#define NO_ALIGNMENT_START -1

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

Cigar::Cigar(const char *s) {
	ASSERT_NOT_NULL(s);
	if(std::strcmp(s,"*")==0) FATAL("cannot handle cigar string '*'");
	size_t i = 0;
	while(s[i]!=0) {
		int len=0;
		while(isdigit(s[i])) {
		    len = len*10 + ((int)s[i]-(int)'0');
		    i++;
		    }
		assert(len>0);
		assert(s[i]!=0);
		CigarElement ce(
			CigarOperator::valueOf(s[i]),
			len
			);
		this->elements.push_back(ce);
		i++;
		}
	}

int Cigar::size() {
	return elements.size();
	}

int Cigar::getReferenceLength() const
    {
    int n=0;
    for(auto ce:elements) {
	if(ce.op->consumeRef) n+=ce.len;
	}
    return n;
    }

int Cigar::getRightClipLength() const
    {
    if(elements.empty()) return 0;
    int n=0;
    size_t i=elements.size()-1;
    while(i>0 && elements[i].op->isClipping())
	{
	n+=elements[i].len;
	i--;
	}
    return n;
    }
int Cigar::getLeftClipLength() const
    {
    int n=0;
    for(size_t i=0; i< elements.size() && elements[i].op->isClipping();i++)
   	{
   	n+=elements[i].len;
   	}
    return n;
    }


CigarElement& Cigar::at(int i)
    {
    return this->elements[i];
    }


#define CASE_INT(c,t) \
	case  c: \
	    this->t##_v = *(t##_t*)s;\
	    s += sizeof(t##_t);\
	    return true
#define float_t float
#define double_t double
#define char_t char

class AuxIterator
{
private:
	const SAMRecord* owner;
	const bam1_core_t *c;
	uint8_t* s;
public:
	char key[3];
	char type;
	char char_v;
	uint8_t uint8_v;
	int8_t int8_v;
	uint16_t uint16_v;
	int16_t int16_v;
	uint32_t uint32_v;
	int32_t int32_v;
	float_t float_v;
	double_t double_v;
	std::string string_v;

	AuxIterator(const SAMRecord* rec):owner(rec),c(&(rec->b->core)),s(0) {
	    s = (uint8_t*)bam_get_aux(rec->b);
	    key[2]=0;
	    type =0;
	    char_v=0;
	    int8_v=0;uint8_v=0;
	    int16_v=0;uint16_v=0;
	    int32_v=0;uint32_v=0;
	    float_v=0;
	    double_v=0;
	    }

	bool next() {
	    if(!(s+4 <= owner->b->data + owner->b->l_data)) return false;
	    string_v.clear();
	    this->key[0] = s[0];
	    this->key[1] = s[1];
	    this->key[2] = 0;
	    s+=2;
	    this->type = *s;
	    s++;
	    switch(this->type) {
		CASE_INT('a',char);
		CASE_INT('c',int8);
		CASE_INT('C',uint8);
		CASE_INT('s',int16);
		CASE_INT('S',uint16);
		CASE_INT('i',int32);
		CASE_INT('I',uint32);
		CASE_INT('f',float);
		CASE_INT('d',double);
		case 'Z': case 'H':
		    while (s < owner->b->data + owner->b->l_data && *s) {
			this->string_v += *s;
			s++;
			}
		    return true;
		case 'B': {
		    //TODO !!
		    WARN("no implemented !!");
		    return false;
		    }
		}
	    return false;
	    }
	};

SAMRecord::SAMRecord(const bam_hdr_t *header, bam1_t *b,bool clone):
	_clone(clone),
	_cigar(0),
	mAlignmentEnd(NO_ALIGNMENT_START),
	header(header),b(0)
	{
	ASSERT_NOT_NULL(b);
	ASSERT_NOT_NULL(header);
	this->b= _clone ?  ::bam_dup1(b) : b;
	ASSERT_NOT_NULL(this->b);
	}

SAMRecord::~SAMRecord() {
	if(_cigar!=0) delete _cigar;
	if(_clone) ::bam_destroy1(this->b);
	}

const bam1_core_t* SAMRecord::cor() const {
	return &(this->b->core);
	}

int SAMRecord::getFlag() const {
	return cor()->flag;
	}
bool SAMRecord::hasFlag(int flg) const {
	return getFlag() & flg;
	}
bool SAMRecord::isPaired() const  {
	return hasFlag(BAM_FPAIRED);
	}
bool SAMRecord::isReadUnmapped() const {
	return hasFlag(BAM_FMUNMAP);
	}
bool SAMRecord::isMateUnmapped() const {
	return hasFlag(BAM_FMUNMAP);
	}
bool SAMRecord::isReverseStrand() const {
	return hasFlag(BAM_FREVERSE);
	}
bool SAMRecord::isMateReverseStrand() const {
	return hasFlag(BAM_FMREVERSE);
	}
bool SAMRecord::isFirstInPair() const {
	return hasFlag(BAM_FREAD1);
	}
bool SAMRecord::isSecondInPair() const {
	return hasFlag(BAM_FREAD2);
	}
bool SAMRecord::isSecondaryAlignment() const {
	return hasFlag(BAM_FSECONDARY);
	}
bool SAMRecord::isSupplementaryAlignment() const {
	return hasFlag(BAM_FSUPPLEMENTARY);
	}
bool SAMRecord::isDuplicate() const {
	return hasFlag(BAM_FDUP);
	}

bool SAMRecord::isFailingQC() const {
	return hasFlag(BAM_FQCFAIL);
	}

bool SAMRecord::isSecondaryOrSupplementaryAlignment() const {
    return isSecondaryAlignment() || isSupplementaryAlignment();
    }

bool SAMRecord::isProperPair() const {
	return hasFlag(BAM_FPROPER_PAIR);
	}

Cigar* SAMRecord::getCigar() {
	if(isReadUnmapped()) return NULL;
	if(_cigar==NULL) _cigar=new Cigar(b);
	return _cigar;
	}
int SAMRecord::getReferenceIndex() const {
	return cor()->tid;
	}
const char* SAMRecord::getReferenceName() {
	int i = getReferenceIndex();
	return i<0 ? 0 : header->target_name[i];
	}
int SAMRecord::getMateReferenceIndex() const {
	return cor()->mtid;
	}
const char* SAMRecord::getMateReferenceName() {
	int i = getMateReferenceIndex();
	return i<0 ? 0 : header->target_name[i];
	}
int SAMRecord::getAlignmentStart() const {
	if(isReadUnmapped()) return NO_ALIGNMENT_START;
	return cor()->pos+1;
	}

int SAMRecord::getMateAlignmentStart() const {
	if(!isPaired()) return NO_ALIGNMENT_START;
	if(isMateUnmapped()) return NO_ALIGNMENT_START;
	return cor()->mpos+1;
	}

int SAMRecord::getStart() const{
	return (const_cast<SAMRecord*>(this))->getAlignmentStart();
	}
int SAMRecord::getEnd() const{
	return (const_cast<SAMRecord*>(this))->getAlignmentEnd();
	}
const char* SAMRecord::getContig() const{
	return (const_cast<SAMRecord*>(this))->getReferenceName();
	}

int SAMRecord::getInferredSize() {
	return cor()->isize;
	}
int SAMRecord::getReadLength() {
	return cor()->l_qseq;
	}
char SAMRecord::getBaseAt(int i)
	{
	if(i<0 || i>=getReadLength()) FATAL("index out of bound i="<<i<< " read len="<< getReadLength());
	uint8_t *s = bam_get_seq(b);
	return "=ACMGRSVTWYHKDBN"[bam_seqi(s, i)];
	}

char SAMRecord::getQualAt(int i)
	{
	uint8_t *s = bam_get_qual(b);
	if (s[0] == 0xff) return '*';
	return s[i] + 33;
	}

int SAMRecord::getAlignmentEnd() {
       if (isReadUnmapped()) {
            return NO_ALIGNMENT_START;
            }
       else if (this->mAlignmentEnd == NO_ALIGNMENT_START) {
	    this->mAlignmentEnd = getAlignmentStart() + getCigar()->getReferenceLength() - 1;
	    }
        return this->mAlignmentEnd;
	}
int SAMRecord::getUnclippedStart() {
    if(isReadUnmapped()) return NO_ALIGNMENT_START;
    return getAlignmentStart() - getCigar()->getLeftClipLength();
    }
int SAMRecord::getUnclippedEnd() {
    if(isReadUnmapped()) return NO_ALIGNMENT_START;
    return getAlignmentEnd() + getCigar()->getRightClipLength();
    }

const char* SAMRecord::getReadName() {
    return bam_get_qname(this->b);
    }

int SAMRecord::getReadNameLength() {
    return cor()->l_qname;
    }

bool SAMRecord::hasAttribute(const char* att_name) const
    {
     AuxIterator iter(this);
     while(iter.next()) {
	 if(strcmp(iter.key,att_name)==0) return true;
	 }
     return false;
    }

 shared_ptr<string> SAMRecord::getMateCigarString() const {
 	return getStringAttribute("MC");
 }


shared_ptr<Cigar> SAMRecord::getMateCigar() const {
    shared_ptr<Cigar> ret;
    shared_ptr<string> cigar_str=getMateCigarString();
    if(!cigar_str) return ret;
    ret.reset(new Cigar(cigar_str->c_str()));
    return ret;
    }

 shared_ptr<string> SAMRecord::getStringAttribute(const char* s) const {
	 shared_ptr<string> ret;
 	 AuxIterator iter(this);
 	 while(iter.next()) {
		 if(iter.type!='Z') continue;
		 if(strcmp(iter.key,s)!=0) continue;
		 ret.reset(new string(iter.string_v));
		 break;
		 }
 	 return ret;
	 }

int SAMRecord::getMateAlignmentEnd() const {
	if(!isPaired()) return NO_ALIGNMENT_START;
	if(isMateUnmapped()) return NO_ALIGNMENT_START;
	shared_ptr<Cigar> cig = this->getMateCigar();
	if(!cig) return NO_ALIGNMENT_START;
	int p= this->getMateAlignmentStart();
	if(p==NO_ALIGNMENT_START) return p;
	return  p + cig->getReferenceLength() - 1;
	}

int SAMRecord::getMateAlignmentEndThenStart() const {
    int n = getMateAlignmentEnd();
    return (n == NO_ALIGNMENT_START ?
	getMateAlignmentStart():
	n
	);
    }
bool SAMRecord::hasMateMappedOnSameReference() const {
    if(this->isReadUnmapped()) return false;
    if(!isPaired()) return false;
    if(this->isMateUnmapped()) return false;
    return this->getReferenceIndex() == this->getMateReferenceIndex();
    }

