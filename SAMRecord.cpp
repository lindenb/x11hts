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
		size_t i = 0;
		while(s[i]!=0) {
			int len=0;
			while(isdit(s[i])) {
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



SAMRecord::SAMRecord(const bam_hdr_t *header, bam1_t *b,bool clone):
	_clone(clone),
	_cigar(0),
	mAlignmentEnd(NO_ALIGNMENT_START),
	header(header),b(0)
	{
	this->b= _clone ?  ::bam_dup1(b) : b;
	}

SAMRecord::~SAMRecord() {
	if(_cigar!=0) delete _cigar;
	if(_clone) ::bam_destroy1(this->b);
	}

bam1_core_t* SAMRecord::cor() {
	return &(this->b->core);
	}

int SAMRecord::getFlag() const {
	return const_cast<SAMRecord*>(this)->cor()->flag;
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
	if(isReadUnmapped()) return NO_ALIGNMENT_START;
	return cor()->pos+1;
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

bool SAMRecord::hasAttribute(const char* s) const_cast
 {
	 AuxIterator iter(this);
	 while(iter.next()) {
		 if(strcmp(iter.key,s)==0) return true;
	 }
	 return false;
 }

 shared_ptr<string> SAMRecord::getMateCigarString() const {
 	return getStringAttribute("MC");
 }


shared_ptr<Cigar> SAMRecord::getMateCigar() const
{
	shared_ptr<Cigar> ret;
	shared_ptr<string> cigar_str=getMateCigarString();
	if(!cigar_str) return ret;
	ret.reset(new Cigar(cigar_str->c_str()));
	return ret;
}

 shared_ptr<string> SAMRecord::getStringAttribute(const char* s) const_cast
  {
	 shared_ptr<string> ret;
 	 AuxIterator iter(this);
 	 while(iter.next()) {
 		 if(strcmp(iter.key,s)!=0) continue;
		 ret.reset(new string(key->s_val));
		 break;
		 }
 	 return ret;
  }

int SAMRecord::getMateAlignmentEnd() const_cast
	 {
		if(!isPaired()) return NO_ALIGNMENT_START;
    if(isMateUnmapped()) return NO_ALIGNMENT_START; 
		shared_ptr<Cigar> cig = this->getMateCigar();
		if(!cig) return NO_ALIGNMENT_START;

		AuxIterator iter(this);
		while(iter.next()) {
			if(strcmp(iter.key,s)!=0) continue;
			ret.reset(new string(key->s_val));
			break;
			}
		return ret;
	 }

class AuxIterator
{
public:
	uint8_t* s;
	code* b;
	char key[3];
	AuxIterator(const SAMRecord* rec):s(bam_get_aux(b)) {
   key[2]=0;
	}
	bool hasNext() {
		return ;
	}
	bool next() {
		std::shared_ptr<AuxKeyValue> ret;
		if(!(s+4 <= b->data + b->l_data)) return false;

		this->key[0] = s[0];
		this->key[1] = s[1];
		this->key[2] = 0;
		s+-2;
		this->type = *s;
		s++;
		switch(ret->type)
		case 'A':
						ret->c = *s;
						++s;
						break;
		case 'C':
						ret->d = *s;
						++s;
						break;
		case 'c': {
						//kputsn("i:", 2, str);
						//kputw(*(int8_t*)s, str);
						++s;
				} else if (type == 'S') {
						if (s+2 <= b->data + b->l_data) {
								//kputsn("i:", 2, str);
								//kputw(*(uint16_t*)s, str);
								s += 2;
						} else return -1;
				} else if (type == 's') {
						if (s+2 <= b->data + b->l_data) {
								//kputsn("i:", 2, str);
								//kputw(*(int16_t*)s, str);
								s += 2;
						} else return -1;
				} else if (type == 'I') {
						if (s+4 <= b->data + b->l_data) {
								//kputsn("i:", 2, str);
								//kputuw(*(uint32_t*)s, str);
								s += 4;
						} else return -1;
				} else if (type == 'i') {
						if (s+4 <= b->data + b->l_data) {
								//kputsn("i:", 2, str);
								//kputw(*(int32_t*)s, str);
								s += 4;
						} else return -1;
				} else if (type == 'f') {
						if (s+4 <= b->data + b->l_data) {
								//ksprintf(str, "f:%g", *(float*)s);
								s += 4;
						} else return -1;

				} else if (type == 'd') {
						if (s+8 <= b->data + b->l_data) {
								//ksprintf(str, "d:%g", *(double*)s);
								s += 8;
						} else return -1;
				} else if (type == 'Z' || type == 'H') {
					  std::string value;

						//kputc(type, str); kputc(':', str);
						while (s < b->data + b->l_data && *s) {
							kputc(*s++, str);
							s++;
						}
						if (s >= b->data + b->l_data)
								return -1;
						if(visitor.()) break;
						++s;
				} else if (type == 'B') {
						uint8_t sub_type = *(s++);
						int sub_type_size = aux_type2size(sub_type);
						uint32_t n;
						if (sub_type_size == 0 || b->data + b->l_data - s < 4)
								return -1;
						memcpy(&n, s, 4);
						s += 4; // now points to the start of the array
						if ((b->data + b->l_data - s) / sub_type_size < n)
								return -1;
						kputsn("B:", 2, str); kputc(sub_type, str); // write the typing
						for (i = 0; i < n; ++i) { // FIXME: for better performance, put the loop after "if"
								kputc(',', str);
								if ('c' == sub_type)      { kputw(*(int8_t*)s, str); ++s; }
								else if ('C' == sub_type) { kputw(*(uint8_t*)s, str); ++s; }
								else if ('s' == sub_type) { kputw(*(int16_t*)s, str); s += 2; }
								else if ('S' == sub_type) { kputw(*(uint16_t*)s, str); s += 2; }
								else if ('i' == sub_type) { kputw(*(int32_t*)s, str); s += 4; }
								else if ('I' == sub_type) { kputuw(*(uint32_t*)s, str); s += 4; }
								else if ('f' == sub_type) { ksprintf(str, "%g", *(float*)s); s += 4; }
								else return -1;
						}
				}
