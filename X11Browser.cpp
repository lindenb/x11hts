/*
The MIT License (MIT)

Copyright (c) 2019 Pierre Lindenbaum PhD.

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in
all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
THE SOFTWARE.

*/
#include <memory>
#include <map>
#include <sstream>
#include <cmath>
#include <fstream>
#include <algorithm>
#include <vector>
#include <memory>
#include <set>
#include <cstdlib>
#include <unistd.h>
#include <getopt.h>
#include "version.hh"
#include "BedLine.hh"
#include "SAMFile.hh"
#include "SAMRecord.hh"
#include "X11Launcher.hh"
#include "Graphics.hh"
#include "Utils.hh"
#include "Faidx.hh"
#include "Locatable.hh"
#include "Interval.hh"

#include "macros.hh"

using namespace std;

#define NO_ARROW 0
#define ARROW_LEFT 1
#define ARROW_RIGHT 2

enum display_base_mode
    {
    display_base,
    display_readname,
    display_none
    };

class X11Browser;

class BrowserInterval:public Locatable
    {
    public:
	string contig;
	int start;
	int end;
	BrowserInterval():contig("NA"),start(0),end(0) {}
	virtual ~BrowserInterval() {}
	virtual const char* getContig() const { return contig.c_str();}
	virtual int getStart() const { return start;}
	virtual int getEnd() const { return end;}

	void assign(BrowserInterval* other) {
	    contig.assign(other->contig);
	    start = other->start;
	    end = other->end;
	}

	int length() const {
	    return this->getLengthOnReference();
	}
    };

class PairOrReads:public Locatable
	{
	public:
		X11Browser* owner;
		SAMRecord* first;
		SAMRecord* second;
		PairOrReads(X11Browser* owner,SAMRecord* rec):owner(owner),first(rec),second(0) {
			ASSERT_NOT_NULL(rec);
		}

		void visit(SAMRecord* rec) {
			second=rec;
		}

		virtual ~PairOrReads() {
			delete first;
			if(second!=0) delete second;
		}

		virtual const char* getContig() const {
		    return first->getContig();
		    }
		virtual int getStart() const;
		virtual int getEnd() const;
	};

static bool compare_pairs(const PairOrReads* p1,const PairOrReads* p2) {
	if(p1->getStart() == p2->getStart()) return p1->getEnd() < p2->getEnd();
	return p1->getStart() < p2->getStart();
}

class X11Browser:public X11Launcher
	{
	private:
		void drawArrow(Graphics& g,const char* fill,const char* stroke,double top_row_y,double feature_height,int refPos,int len,int arrow);
		char getBaseAt(SAMRecord* rec,int pos);

	public:
		std::vector<SAMFile*> bams;
		std::vector<BrowserInterval*> intervals;
		std::vector<vector<PairOrReads*>*> rows;
		BrowserInterval* interval;
		size_t bam_index;
		size_t interval_idx;
		bool group_by_pair;
		bool show_clip;
		int show_base;
		IndexedFastaSequence* indexedFastaSequence;
		std::shared_ptr<std::string> reference_seq;
		bool hide_supplementary_reads;
		bool hide_duplicate_reads;
		bool hide_secondary_reads;
		bool hide_fail_qc_reads;
		size_t top_row_index;


		X11Browser();
		virtual ~X11Browser();
		virtual double distance2pixel(int genomic_len);
		virtual double pos2pixel(int pos);
		virtual int pixel2pos(double pixel);
		virtual int getStart( SAMRecord* rec);
		virtual int getEnd( SAMRecord* rec);
		virtual int doWork(int argc,char** argv);
		virtual void repaint();
		virtual void paint();
		virtual void paint(Graphics& g);
		double trimx(double v) { return std::max(0.0,std::min(v,(double)this->window_width));}
		void doZoom(double f);
		void doMove(double f);
	};

X11Browser::X11Browser():interval(0),
	bam_index(0),
	interval_idx(0),
	group_by_pair(true),
	show_clip(true),
	show_base(display_base),
	indexedFastaSequence(0),
	hide_supplementary_reads(true),
	hide_duplicate_reads(true),
	hide_secondary_reads(true),
	hide_fail_qc_reads(true),
	top_row_index(0UL)
    {
    interval = new BrowserInterval;
    interval->contig.assign("undef");
    interval->start=1;
    interval->end=1;

    app_desc="X11 based bam viewer";
    app_name="browse";



    Option* opt= new Option('D',true,"Export directory; Where to save screenshots.");
    opt->arg("directory");
    options.push_back(opt);

    }

X11Browser::~X11Browser() {
	if(indexedFastaSequence!=0) delete indexedFastaSequence;
	for(auto bam:bams) delete bam;
	for(auto r:intervals) delete r;
	if(interval!=0) delete interval;
	for(size_t i=0;i< rows.size();++i){
		for(size_t j=0;j< rows[i]->size();++j) delete rows[i]->at(j);
		delete rows[i];
	}
}

int X11Browser::getStart( SAMRecord* rec) {
    return this->show_clip?
	    rec->getUnclippedStart():
	    rec->getAlignmentStart()
	    ;
}
int X11Browser::getEnd( SAMRecord* rec) {
    return this->show_clip?
	    rec->getUnclippedEnd():
	    rec->getAlignmentEnd()
	    ;
    }

double X11Browser::distance2pixel(int genomic_len)
    {
    return (genomic_len)/((double)interval->length())* window_width;
    }

double X11Browser::pos2pixel(int pos) {
	return distance2pixel(pos-interval->start);
}

int X11Browser::pixel2pos(double pix) {
	return interval->start+ (pix)/((double)window_width)* interval->length();
}

void X11Browser::drawArrow(
	Graphics& g,
	const char* fill,
	const char* stroke,
	double top_row_y,double feature_height,int refPos,int len,int arrow
	)
    {

    double width = distance2pixel(len);
    double arrow_size = (arrow==NO_ARROW?0:5);
    if(arrow_size > feature_height) arrow=NO_ARROW;
    if(arrow_size>= width )  arrow=NO_ARROW;

    if(pos2pixel(refPos) > this->window_width) return;
    if(pos2pixel(refPos+len) <0) return;
	

    if(arrow==ARROW_LEFT)
	{
	double x_array[5];
	double y_array[5];


	x_array[0] = pos2pixel(refPos) + width;
	y_array[0] = top_row_y;

	x_array[1] = x_array[0];
	y_array[1] = y_array[0] + feature_height;

	x_array[2] = pos2pixel(refPos)+arrow_size;
	y_array[2] = y_array[1];

	x_array[3] = pos2pixel(refPos);
	y_array[3] = y_array[0] + feature_height/2.0;

	x_array[4] = x_array[2];
	y_array[4] = y_array[0];

	g.setColorName(fill);
	g.fillPolygon(5, x_array, y_array);
	g.setColorName(stroke);
	g.drawPolygon(5, x_array, y_array);
	}
    else if(arrow==ARROW_RIGHT)
    	{
	double x_array[5];
	double y_array[5];

	x_array[0] = pos2pixel(refPos);
	y_array[0] = top_row_y;

	x_array[1] = x_array[0] + width - arrow_size;
	y_array[1] = y_array[0];

	x_array[2] = x_array[0] + width ;
	y_array[2] = y_array[0] + feature_height/2.0;

	x_array[3] = x_array[1];
	y_array[3] = y_array[0] + feature_height;

	x_array[4] = x_array[0];
	y_array[4] = y_array[3];

	g.setColorName(fill);
	g.fillPolygon(5, x_array, y_array);
	g.setColorName(stroke);
	g.drawPolygon(5, x_array, y_array);
    	}
    else //NO_ARROW
       	{


	g.setColorName(fill);
	g.fillRect(pos2pixel(refPos), top_row_y	, width, feature_height);
	g.setColorName(stroke);
	g.drawRect(pos2pixel(refPos), top_row_y	, width, feature_height);
       	}
    }

char X11Browser::getBaseAt(SAMRecord* rec,int pos) {
    char c= rec->getBaseAt(pos);
    return c;
    }
void X11Browser::paint() {
	Pixmap offscreen = getOffscreen();
    X11Graphics g(this->display,offscreen);
    
    paint(g);
    GC stdgc=XDefaultGC(display,screen_number); // "This GC should never be freed"
    ::XCopyArea(display, offscreen, window,stdgc, 0, 0, this->window_width, this->window_height,0, 0);
    ::XFlush(display);
    }

void X11Browser::paint(Graphics& graphics) {
    graphics.setColorName("white");
    graphics.fillRect(0, 0, this->window_width, this->window_height);
    double y=0;
    double font_size = 7;
    /* title */
	{
	ostringstream os;

	os << this->bams[this->bam_index]->getSample() << " ";

	os << this->interval->contig
		<< ":"
		<< Utils::niceInt(this->interval->start)
		<< "-"
		<< Utils::niceInt(this->interval->end)
		<< " length:" <<  Utils::niceInt(this->interval->length())
		<< " (" << (bam_index+1) << "/" << bams.size()<<") "
		<< " (" << (interval_idx+1) << "/" << intervals.size()<<") ";
	string title(os.str());

	double height=font_size+2;
	double width = std::min(this->window_width,(int)(font_size*title.length()));
	graphics.setGray(0.3);
	graphics.drawText(title.c_str(),
		this->window_width/2.0-width/2.0,
		y+1,
		width,
		height-2
		);
	y+=height;

	graphics.drawLine(0, y, this->window_width,y);
	}
    /* ruler label */
    {
    vector<double> verticals;
    int prev_pos=interval->start;
    int x = 0;
    double height=font_size+2;
    graphics.setGray(0.3);
    while(x<=this->window_width)  {
	if(pixel2pos(x)<=prev_pos) {++x;continue;}
	string label(Utils::niceInt(pixel2pos(x)));
	double width = label.size()*font_size ;
	if(x+width > this->window_width) break;
	graphics.drawText(
	    label.c_str(),
	    x+1,
	    y+1,
	    width,
	    height-2
	    );
	verticals.push_back(x);
	prev_pos=pixel2pos(x);
	x+=width+font_size;
	}
    for(auto lx:verticals) {
	graphics.drawLine(lx, y, lx, this->window_height-y);
	}
    y+=height;
    graphics.drawLine(0, y, this->window_width,y);
    }
    /* reference */
    if(this->reference_seq)
	{
	double onebase=distance2pixel(1);
	if(onebase > font_size) {
	    int p= interval->start;
	    while(p <= interval->end)  {
		 size_t gpos = (size_t)(p - this->interval->start);
		 char base='N';
		 if(gpos< this->reference_seq->size())
		     {
		     base = this->reference_seq->at((size_t)gpos);
		     }

		 double w= min(font_size,onebase);
		 graphics.setColorForBase(base);
		 graphics.drawChar(base,pos2pixel(p)+onebase/2.0-w/2.0, y, w,font_size);
		 p++;
		 }
	    y+=font_size;
	    graphics.drawLine(0, y, this->window_width,y);
	    y++;
	    }
	}

    /** DEPTH **/
    double depth_height=100;
    map<int,int> pos2coverage;
    if(this->interval->length() > 50000) depth_height=0;
    double bottom_depth=y+depth_height;
    y=bottom_depth+1;

    NullGraphics nullg;


    /** each row */
    double feature_height= std::max(10.0,std::min(30.0,distance2pixel(1)));
    for(size_t y_row=0;y_row< rows.size() ;++y_row) {
	double top_row_y = y;
	double midy= top_row_y + feature_height /2.0;
	/* current row */
	vector<PairOrReads*>* row=rows[y_row];
	/* we neeed to loop over all the lines even if they're not visible to calculate the depth */
	bool row_visible = y_row>= this->top_row_index &&  y< this->window_height;

	Graphics& g=(row_visible?graphics:nullg);

	/** all elements on this row */
	for(size_t por_idx=0;por_idx<row->size();++por_idx) {
	    PairOrReads* por = row->at(por_idx);

	    /* draw a line for the pair segment */
	    if(this->group_by_pair && por->second!=0)
		{
		if(por->first->isPaired() && !por->first->isProperPair()){
		    g.setColorName("red");
		    }
		else
		    {
		    g.setColorName("blue");
		    }
		double x0=trimx(pos2pixel(por->getStart()));
		double x1=trimx(pos2pixel(por->getEnd()+1));
		g.drawLine( x0, midy, x1, midy );
		/* can we plot the length of the insert ? */

		// enough space to draw the insert size ?
		if(x1-x0>100) {
		    string title = Utils::niceInt(abs(por->first->getInferredSize()));
		    double w = std::min(title.size()*7.0,x1-x0);
		    g.drawText(title.c_str(),
			    x0+(x1-x0)/2.0- w/2.0,
			    midy-3,
			    w,
			    7);
		    }

		}
	    // each side of the read pair
	    for(int side=0;side<2;++side) {
		SAMRecord* rec = (side==0?por->first : por->second);
		if(rec==NULL) continue;
		set<double> insertions_x;
		int refPos = rec->getUnclippedStart();
		int readPos = 0;
		Cigar* cigar = rec->getCigar();

		g.setColorName("black");
		/* rec line */
		g.drawLine(
			trimx(pos2pixel(rec->getUnclippedStart())),
			midy,
			trimx(pos2pixel(rec->getUnclippedEnd()+1)),
			midy
		    );

		// loop over read
		for(int cigar_index=0;cigar_index < cigar->size();cigar_index++) {
		    CigarElement& ce = cigar->at(cigar_index);
		    int arrow= NO_ARROW;

		    /* shall we draw an arrow ? */
		    if(rec->isReverseStrand() && !this->show_clip) {
			if(cigar_index==0 && ce.op->isMatch()) arrow  = ARROW_LEFT;
			else if(cigar_index>0 &&  cigar->at(cigar_index-1).op->isClipping()) arrow  = ARROW_LEFT;
			}
		    else if(rec->isReverseStrand() && cigar_index==0 && this->show_clip )
			{
			arrow  = ARROW_LEFT;
			}
		    else if(rec->isReverseStrand() && !this->show_clip) {
			if(cigar_index+1==cigar->size() && ce.op->isMatch()) arrow  = ARROW_RIGHT;
			else if(cigar_index+1<cigar->size() &&  cigar->at(cigar_index+1).op->isClipping()) arrow  = ARROW_LEFT;
			}
		    else  if(!rec->isReverseStrand() && cigar_index+1==cigar->size() && this->show_clip)
			{
			arrow  = ARROW_RIGHT;
			}

		    switch(ce.op->op) {
			case 'P':break;
			case 'I':
			    {
			    insertions_x.insert(pos2pixel(refPos));
			    readPos+=ce.len;
			    break;
			    }
			case 'N':
			case 'D':
			    {
			    double x0=trimx(pos2pixel(refPos));
			    double x1=trimx(pos2pixel(refPos+ce.len));
			    // enough space to draw the insert size ?
			    if(x1-x0>100) {
				ostringstream os;
				os << Utils::niceInt(ce.len) << "bp";
				string title = os.str();
				g.setColorName("pink");
				double w = std::min(title.size()*7.0,x1-x0);
				g.drawText(title.c_str(),
					x0+(x1-x0)/2.0- w/2.0,
					midy-3,
					w,
					7);
				}
			    refPos+=ce.len;
			    break;
			    }
			case 'H':
			case 'S':
			case 'X':
			case '=':
			case 'M':
			    {
			    /* don't print clip */
			    if(!show_clip && ce.op->isClipping()) {
				refPos += ce.len;
				if(ce.op->op!='H') readPos += ce.len;
				break;
				}



			    char const * fill;
			    char const * stroke;
			    if(ce.op->isClipping())
				{
				fill  = "yellow";
				}
			    else if(ce.op->op=='X')
				{
				fill = "red";
				}
			    else
				{
				fill = "gray";
				}
			    if(rec->isPaired() && (rec->isMateUnmapped() || !rec->isProperPair())){
				stroke="orange";
				}
			    else
				{
				stroke="black";
				}


			    drawArrow(
				   g,
				   fill,
				   stroke,
				   top_row_y,
				   feature_height,
				   refPos,
				   ce.len,
				   arrow
				   );
			    /* draw bases */
			    double one_base=distance2pixel(1);
			    if(ce.op->op!='H' ) {
				for(int t=0;t< ce.len ;++t) {
				    int basepos = refPos+t;
				    if(basepos< this->interval->start) continue;
				    if(basepos> this->interval->end) break;

				    if(!ce.op->isClipping() /* don't use clipped base for coverage */
					  && depth_height>0 /* there is coverage */) {
					/* add this base for coverage */
					pos2coverage[basepos]++;
				    }

				    char nucleotide = rec->getBaseAt(readPos+t);
				    char base= nucleotide;

				    if(this->show_base==display_readname)
					{
					base = readPos+t >= rec->getReadNameLength()?' ': rec->getReadName()[readPos+t];
					base= rec->isReverseStrand()?tolower(base):toupper(base);
					}
				    double width=std::min(feature_height,one_base);
				    bool print_base= this->show_base!=display_none;

				    if(this->reference_seq &&
					(basepos >= this->interval->start) &&
					(basepos - this->interval->start) < (int)this->reference_seq->size()) {
					char ref_base = toupper(this->reference_seq->at(basepos - this->interval->start));
					if(ref_base!='N' && nucleotide!='N' && ref_base!=nucleotide)
					    {
					    g.setColorName("red");
					    g.fillRect(pos2pixel(basepos), top_row_y, one_base, feature_height);
					    print_base=true;
					    }
					}
				    if(one_base<1) print_base=false;

				    //this->show_base!=display_none && one_base >4)
				    if(print_base) {
					g.setColorForBase(base);
					g.drawChar(base, pos2pixel(basepos)+one_base/2.0-width/2.0, top_row_y+2, width, feature_height-4);
					}
				    }
				}
			    refPos+=ce.len;
			    if(ce.op->op!='H') readPos+=ce.len;
			    break;
			    }
			default:FATAL("unknown operator");break;
			} // end swith cigar op
		    }// end loop over cigar
		/* print insertions */
		for(auto ins_x:insertions_x)
		    {
		    g.setColorName("red");
		    g.fillRect(ins_x, top_row_y, 1, feature_height);
		    }

		}
	    }
	   if(row_visible ) y+=feature_height+2;
	}/* end for each-row */

    /* print depth of coverage */
    if(depth_height>1) {
	int max_cov=1;
	for(auto p:pos2coverage) {
	    max_cov=std::max(max_cov,p.second);
	    }
	double* x_array=new double[2+this->window_width];
	double* y_array=new double[2+this->window_width];

	x_array[0]=this->window_width;
	y_array[0]=bottom_depth;

	x_array[1]=0;
	y_array[1]=y_array[0];

	for(int i=0;i< this->window_width;i++)
	    {
	    int g1 = pixel2pos(i+0);
	    int g2 = pixel2pos(i+1);
	    int n=0;
	    double t=0.0;
	    while(g1<=g2) {
		n++;
		t+= pos2coverage[g1];
		++g1;
		}
	    double v=t/n;
	    x_array[2+i]=i;
	    y_array[2+i]= bottom_depth - depth_height*( v/(double)max_cov);
	    }
	graphics.setGray(0.3);
	graphics.fillPolygon(2+this->window_width, x_array, y_array);
	delete[] x_array;
	delete[] y_array;
	}

    }
void X11Browser::repaint() {

	for(size_t i=0;i< rows.size();++i){
		for(size_t j=0;j< rows[i]->size();++j) delete rows[i]->at(j);
		delete rows[i];
		}
	rows.clear();
	vector<PairOrReads*> all_pairs;
	std::map<std::string,PairOrReads*> name2pair;
	SAMFile* bam = this->bams[this->bam_index];
	int tid = bam->contigToTid(interval->contig.c_str());

	/* reload reference sequence */
	this->reference_seq.reset();
	if(this->indexedFastaSequence!=0 && this->interval->length()< 2*this->window_width)
	    {
	    this->reference_seq = this->indexedFastaSequence->fetch(
		    this->interval->contig.c_str(),
		    this->interval->start-1,
		    this->interval->end
		    );
	    }


	if(tid>=0) {
		int ret;
		bam1_t *b = ::bam_init1();
		int extend=(this->show_clip||this->group_by_pair?500:0);

		hts_itr_t *iter = ::sam_itr_queryi(bam->idx, tid,
				std::max(1,interval->start- extend),
				interval->end+extend
				);
		while ((ret = bam_itr_next(bam->fp, iter, b)) >= 0)
			{
			SAMRecord* rec = new SAMRecord(bam->hdr,b,true);
			if(rec->isReadUnmapped() ||
				rec->getReferenceIndex()!=tid ||
				(this->hide_fail_qc_reads && rec->isFailingQC()) ||
				(this->hide_duplicate_reads && rec->isDuplicate())
				)
			    {
			    delete rec;
			    continue;
			    }

			if(!group_by_pair)
			    {
			    /* check flags */
			    if(
			    	(this->hide_supplementary_reads && rec->isSupplementaryAlignment()) ||
			    	(this->hide_secondary_reads && rec->isSecondaryAlignment()) 
				) {
			    	delete rec;
			    	continue;
			    	}
	
			    
			    
			    PairOrReads* po= new PairOrReads(this,rec);
			    if(!po->overlaps(interval)) {
					delete po;
					continue;
					}
				
			    all_pairs.push_back(po);
			    continue;
			    }



			if(rec->isSecondaryOrSupplementaryAlignment())
			    {
			    delete rec;
			    continue;
			    }

			string read_name(rec->getReadName());
			map<string,PairOrReads*>::iterator iter= name2pair.find(read_name);
			if(iter==name2pair.end()) {
				PairOrReads* por = new PairOrReads(this,rec);
				name2pair.insert(make_pair(read_name,por));
				}
			else if(iter->second->second==0)
				{
				iter->second->visit(rec);
				}
			else
				{
				WARN("duplicate read name " << read_name );
				delete rec;
				}
			}
		::hts_itr_destroy(iter);
		::bam_destroy1(b);
		}

	for(map<string,PairOrReads*>::iterator iter= name2pair.begin();
			iter!=name2pair.end();
			++iter)
		{
		PairOrReads* por = iter->second;
	
		/*as we extend the region when fetching the reads, make sure that NOT(bot reads hidden )*/
		if( (por->second!=NULL && getEnd(por->first) < interval->getStart() && getStart(por->second) > interval->getEnd()) ||
		    (por->second!=NULL && getEnd(por->second) < interval->getStart() && getStart(por->first) > interval->getEnd()) 
		   )    {
			delete por;
			continue;
			}

		all_pairs.push_back(por);
		}

	std::sort(all_pairs.begin(),all_pairs.end(),compare_pairs);
	for(size_t i=0;i< all_pairs.size();++i) {
		PairOrReads* por = all_pairs[i];
		size_t y_row=0UL;
		while(y_row< this->rows.size()) {
			vector<PairOrReads*>* row=this->rows[y_row];
			PairOrReads* last = row->at(row->size()-1);
			if(last->getEnd()+1 < por->getStart()) {
				row->push_back(por);
				break;
				}
			y_row++;
			}
		if(y_row==this->rows.size()) {
			vector<PairOrReads*>* row=new vector<PairOrReads*>;
			row->push_back(por);
			rows.push_back(row);
			}
		}
	paint();
	}

void X11Browser::doZoom(double factor) {
    const double len  = this->interval->length();
    if(len<=1.0) return;
    double new_len= len*factor;
    if(new_len==len && factor>1.0) new_len=len+1;
    if(new_len==len && factor<1.0) new_len=len-1;
    if(new_len<0) return;
    int mid = this->interval->start+(int)(len/2.0);
    this->interval->end = mid + (int)(new_len/2.0);
    this->interval->start = std::max(1,this->interval->end - (int)new_len);
   
    }



void X11Browser::doMove(double factor) {
    const int len  = this->interval->length();
    int dlen= len*abs(factor);
    if(dlen<1) dlen=1;
    int x=  this->interval->start + (factor<0.0?-1:1)*dlen;
    if(x<1) x=1;
    this->interval->start = x;
    this->interval->end = x + len -1;
    }


int X11Browser::doWork(int argc,char** argv) {
	int id_generator = 0;
	const KeyAction* actionQuit = createKeyAction(XKEY_STR(XK_Q), "Exit.");
	const KeyAction* actionEscape = createKeyAction(XKEY_STR(XK_Escape), "Exit.");
	const KeyAction* actionNextInterval = createKeyAction(XKEY_STR(XK_L), "Move to next interval.");
	const KeyAction* actionPrevInterval = createKeyAction(XKEY_STR(XK_H), "Move to previous interval.");
	const KeyAction* actionResetInterval = createKeyAction(XKEY_STR(XK_I), "Restore current interval.");
	const KeyAction* actionNextBam = createKeyAction(XKEY_STR(XK_J), "Move to next bam.");
	const KeyAction* actionPrevBam = createKeyAction(XKEY_STR(XK_K), "Move to previous bam.");
	const KeyAction* actionZoomIn = createKeyAction(XKEY_STR(XK_plus), "Zoom In");
	const KeyAction* actionZoomOut = createKeyAction(XKEY_STR(XK_minus), "Zoom Out");
	const KeyAction* actionMoveLeft = createKeyAction(XKEY_STR(XK_Left), "Move left");
	const KeyAction* actionMoveRight = createKeyAction(XKEY_STR(XK_Right), "Move Right");
	const KeyAction* actionShowBase = createKeyAction(XKEY_STR(XK_B), "Toggle: Show Base/ No Base / Read Name");
	const KeyAction* actionExportPS = createKeyAction(XKEY_STR(XK_P), "Export current view to Postscript");
	const KeyAction* actionExportSVG = createKeyAction(XKEY_STR(XK_S), "Export current view to SVG");
	const KeyAction* actionShowClip = createKeyAction(XKEY_STR(XK_C), "Show/Hide clip");
	const KeyAction* actionGroupByPair = createKeyAction(XKEY_STR(XK_G), "Group by pair");
	const KeyAction* actionHideSupplementaryReads = createKeyAction(XKEY_STR(XK_1), "Show/Hide supplementary reads (always  hide in 'group by pair'");
	const KeyAction* actionHideSecondaryReads = createKeyAction(XKEY_STR(XK_2), "Show/Hide secondary reads (always  hide in 'group by pair'");
	const KeyAction* actionHideDuplicateReads = createKeyAction(XKEY_STR(XK_3), "Show/Hide duplicate reads.");
	const KeyAction* actionHideFailQCReads = createKeyAction(XKEY_STR(XK_4), "Show/Hide failing QC reads.");
	const KeyAction* actionBamRowUp = createKeyAction(XKEY_STR(XK_E), "Move view up");
	const KeyAction* actionBamRowDown = createKeyAction(XKEY_STR(XK_D), "Move view down");

		
#define ELSE_TOGGLE_KEY(K,B) else if(K->match(evt)) { this->B=!(this->B); repaint();}


	char* export_dir = NULL;
	int opt;

	if(argc<=1) {
		usage(cerr);
		return EXIT_FAILURE;
		}
	std::string opt_str=  this->build_getopt_str();
	while ((opt = getopt(argc, argv, opt_str.c_str())) != -1) {
		switch (opt) {
		case 'D':
		    export_dir = optarg;
		    break;
		default: DEFAULT_HANDLE_OPTION(optopt);break;
		}
	}

	SAMFileFactory samFileFactory;
	while(optind<argc) {
	    char* fname=argv[optind++];
	    if(Utils::endsWith(fname, ".fa") || Utils::endsWith(fname, ".fasta")) {
		if(this->indexedFastaSequence!=NULL) FATAL("Reference defined twice");
		this->indexedFastaSequence = new IndexedFastaSequence(fname);
		}
	    else if(Utils::endsWith(fname, ".bam")) {
		SAMFile* bamFile = samFileFactory.open(fname);
		this->bams.push_back(bamFile);
		}
	    else if(Utils::endsWith(fname, ".list")) {
		ifstream bamin(fname);
		if(!bamin.is_open()) FATAL("Cannot open " << fname << " "<< strerror(errno));
		string line;
		while(getline(bamin,line)) {
			if(Utils::isBlank(line) || line[0]=='#') continue;
			SAMFile* bamFile = samFileFactory.open(line.c_str());
			this->bams.push_back(bamFile);
			}
		bamin.close();
		}
	    else if(Utils::endsWith(fname, ".bed")) {
		string line;
		BedCodec bedCodec;
		ifstream bedin(fname);
		std::vector<string> tokens;
		if(!bedin.is_open()) FATAL("Cannot open BED " << fname << " "<< strerror(errno));
		while(getline(bedin,line)) {
		    if(bedCodec.is_ignore(line)) continue;
		    BrowserInterval* interval = new BrowserInterval;
		    Utils::split('\t',line,tokens);
		    if(!bedCodec.parse(tokens,&interval->contig,&interval->start,&interval->end)) {
			delete interval;
			FATAL("Cannot parse " << line);
			}
		    interval->start++;
		    if(interval->start > interval->end) {
			WARN("Ignore " << line);
			delete interval;
			continue;
			}
		    intervals.push_back(interval);
		    }
		bedin.close();
		}
	    else // consider interval
		{
		std::shared_ptr<Interval> rgn =  Interval::parse(fname);
		if(!rgn) FATAL("Cannot parse interval "<< fname);
		BrowserInterval* interval = new BrowserInterval;
		interval->contig.assign(rgn->getContig());
		interval->start = rgn->getStart();
		interval->end = rgn->getEnd();
		if(interval->start > interval->end) {
		    WARN("Ignore " << fname);
		    delete interval;
		    continue;
		    }
		intervals.push_back(interval);
		}
	    }

	if(this->bams.empty()) FATAL("List of bams is empty.");
	if(this->intervals.empty())  FATAL("List of regions is empty.");
	this->interval->assign(this->intervals[0]);

	//
	this->createWindow();
	//main loop
	XEvent evt;
	bool done=false;
	while(!done) {
		::XNextEvent(this->display, &evt);
		if(actionQuit->match(evt) || actionEscape->match(evt)) {
		    done = true;
		    }
		else if(actionPrevBam->match(evt)) {
		   this-> bam_index = (this->bam_index==0UL?this->bams.size()-1:this->bam_index-1);
                    this->top_row_index=0UL;
		    repaint();
		    }
		else if(actionNextBam->match(evt)) {
		    this->bam_index = (this->bam_index+1>=this->bams.size()?0:this->bam_index+1);
                    this->top_row_index=0UL;
		    repaint();
		    }
		else if(actionPrevInterval->match(evt)) {
		    interval_idx = (interval_idx==0UL?intervals.size()-1:interval_idx-1);
		    this->interval->assign(this->intervals.at(interval_idx));
                    this->top_row_index=0UL;
		    repaint();
		    }
		else if(actionNextInterval->match(evt)) {
		    interval_idx = (interval_idx+1>=intervals.size()?0:interval_idx+1);
		    this->interval->assign(this->intervals.at(interval_idx));
                    this->top_row_index=0UL;
		    repaint();
		    }
		else if(actionResetInterval->match(evt)) {
		    this->interval->assign(this->intervals.at(interval_idx));
		    this->top_row_index=0UL;
		    repaint();	
		    }
		else if(actionZoomIn->match(evt)) {
		    doZoom(0.66);
		    repaint();
		    }
		else if(actionZoomOut->match(evt)) {
		    this->top_row_index=0UL;
		    doZoom(1.3);
		    repaint();
		    }
		else if(actionMoveLeft->match(evt)) {
		    doMove(-0.3);
		    repaint();
		    }
		else if(actionMoveRight->match(evt)) {
		    doMove( 0.3);
		    repaint();
		    }
		else if(actionShowBase->match(evt)) {
		    switch(this->show_base)
			{
			case display_base : this->show_base = display_readname;break;
			case display_readname : this->show_base = display_none;break;
			default: this->show_base = display_base; break;
			}
		    repaint();
		    }
		else if(actionBamRowUp->match(evt)) {
			if(this->top_row_index>0) this->top_row_index--;
			paint();
			}
		else if(actionBamRowDown->match(evt)) {
			this->top_row_index++;
			paint();
			}
		else if(actionGroupByPair->match(evt)) {
			this->top_row_index=0UL;
			this->group_by_pair = !this->group_by_pair;
			repaint();
			}
		ELSE_TOGGLE_KEY(actionShowClip,show_clip)
		ELSE_TOGGLE_KEY(actionHideSupplementaryReads,hide_supplementary_reads)
		ELSE_TOGGLE_KEY(actionHideDuplicateReads,hide_duplicate_reads)
		ELSE_TOGGLE_KEY(actionHideSecondaryReads,hide_secondary_reads)
		ELSE_TOGGLE_KEY(actionHideFailQCReads,hide_fail_qc_reads)
		else if(actionExportPS->match(evt) || actionExportSVG->match(evt))
		    {
		    if(Utils::isBlank(export_dir)) {
			WARN("export directory is not defined.");
			}
		    else
			{
			ostringstream os;
			os << export_dir << (Utils::endsWith(export_dir,"/")?"":"/")
			   << interval->contig<<"_" << interval->start << "_" << interval->end << "."
			    << this->bams.at(bam_index)->getSample()
			    << "." << (++id_generator) << "."
			    << (actionExportPS->match(evt)?"ps":"svg");
			std::string title =os.str();

			ostringstream os2;
			os2 << interval->contig<<"_" << interval->start << "_" << interval->end << "."
				<< this->bams.at(bam_index)->getSample();
			std::string title2 =os2.str();

			unique_ptr<Graphics> g(actionExportPS->match(evt)?
				(Graphics*)new PSGraphics(title.c_str(),window_width,window_height):
				(Graphics*)new SVGraphics(title.c_str(),title2.c_str(),window_width,window_height)
				);
			paint(*g);
			}
		    }
		else if(evt.type ==   Expose)
		    {
		    resized();
		    }
		else  if(evt.type ==  KeyPress) {
		    cerr << "Unknown key" << endl;
		    key_usage(cerr);
		}
		}//end while
	disposeWindow();
	return EXIT_SUCCESS;
	}


int PairOrReads::getStart() const{
    int i = this->owner->getStart(first);
    if(!this->owner->group_by_pair)
    	{
    	return i;
    	}
    else if( this->second!=0 ) {
	assert(strcmp(first->getReadName(),second->getReadName())==0);
	assert(this->first->hasMateMappedOnSameReference());

	i=std::min(i,this->owner->getStart(second));
	assert(i>0);
	}
    else if(this->first->hasMateMappedOnSameReference())
	{
	int j = this->first->getMateAlignmentStart();
   	if(j>0) i= std::min(i,j);
	}
    return i;
    }

int PairOrReads::getEnd() const {
    int i =  this->owner->getEnd(first);
    if(!this->owner->group_by_pair)
	{
	return i;
	}
    else if(second!=0 ) {
	assert(strcmp(first->getReadName(),second->getReadName())==0);
	assert(this->first->hasMateMappedOnSameReference());
	i=std::max(i, this->owner->getEnd(second));
	assert(i>0);
	}
    else if(this->first->hasMateMappedOnSameReference())
   	{
   	int j = this->first->getMateAlignmentStart();//TODO use 'MC:Z'
   	if(j>0) i= std::min(i,j);
   	}
    return i;
    }


int main_browser(int argc,char** argv) {
    X11Browser app;
    return app.doWork(argc,argv);
    }
