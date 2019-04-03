#include <map>
#include <cmath>
#include <fstream>
#include <algorithm>
#include <vector>
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

class Interval
    {
    public:
	string contig;
	int start;
	int end;

	void assign(Interval* other) {
	    contig.assign(other->contig);
	    start = other->start;
	    end = other->end;
	}

	int length() const {
	    return 1+ (end-start);
	}
    };

class PairOrReads
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

		~PairOrReads() {
			delete first;
			if(second!=0) delete second;
		}
		int getStart() const{
			int i = first->getUnclippedStart();
			if(second!=0 ) {
				i=std::min(i,second->getUnclippedStart());
			}
			return i;
		}
		int getEnd() const {
			int i = first->getUnclippedEnd();
			if(second!=0 ) {
				i=std::max(i,second->getUnclippedEnd());
			}
			return i;
		}
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
		std::vector<Interval*> intervals;
		std::vector<vector<PairOrReads*>*> rows;
		Interval* interval;
		size_t bam_index;
		size_t interval_idx;
		bool group_by_pair;
		int show_base;
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
		void zoomIn();
		void zoomOut();
		void goLeft(double f);
		void goRight(double f);
	};

X11Browser::X11Browser():interval(0),
	bam_index(0),
	interval_idx(0),
	group_by_pair(true),
	show_base(display_base) {
    interval = new Interval;
    interval->contig.assign("undef");
    interval->start=1;
    interval->end=1;

    app_desc="X11 based bam viewer";
    app_name="browse";

    Option* opt= new Option('B',true,"A file containing the path to the indexed bam files. One per line");
    opt->required();
    opt->arg("file");
    options.push_back(opt);

    opt= new Option('R',true,"A bed file containing the regions to observe");
    opt->required();
    opt->arg("file.bed");
    options.push_back(opt);

    }

X11Browser::~X11Browser() {
	for(auto bam:bams) delete bam;
	for(auto r:intervals) delete r;
	if(interval!=0) delete interval;
	for(size_t i=0;i< rows.size();++i){
		for(size_t j=0;j< rows[i]->size();++j) delete rows[i]->at(j);
		delete rows[i];
	}
}

int X11Browser::getStart( SAMRecord* rec) {
    return rec->getAlignmentStart();
}
int X11Browser::getEnd( SAMRecord* rec) {
    return rec->getAlignmentEnd();
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

	g.setColor(fill);
	g.fillPolygon(5, x_array, y_array);
	g.setColor(stroke);
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

	g.setColor(fill);
	g.fillPolygon(5, x_array, y_array);
	g.setColor(stroke);
	g.drawPolygon(5, x_array, y_array);
    	}
    else //NO_ARROW
       	{
	g.setColor(fill);
	g.fillRect(pos2pixel(refPos), top_row_y	, width, feature_height);
	g.setColor(stroke);
	g.drawRect(pos2pixel(refPos), top_row_y	, width, feature_height);
       	}
    }

char X11Browser::getBaseAt(SAMRecord* rec,int pos) {
    char c= rec->getBaseAt(pos);
    return c;
    }

void X11Browser::paint() {
    Graphics g(this->display,this->window);
    g.setColor("white");
    g.fillRect(0, 0, this->window_width, this->window_height);
    double y=0;
    double font_size = 7;
    /* title */
    {
	ostringstream os;
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
	g.setColor(0.3);
	g.drawText(title.c_str(),
		this->window_width/2.0-width/2.0,
		y+1,
		width,
		height-2
		);
	y+=height;

	g.drawLine(0, y, this->window_width,y);
	}
    /* ruler label */
    {
    vector<double> verticals;
    int prev_pos=interval->start;
    int x = 0;
    double height=font_size+2;
    g.setColor(0.3);
    while(x<=this->window_width)  {
	if(pixel2pos(x)<=prev_pos) {++x;continue;}
	string label(Utils::niceInt(pixel2pos(x)));
	double width = label.size()*font_size ;
	if(x+width > this->window_width) break;
	g.drawText(
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
	g.drawLine(lx, y, lx, this->window_height-y);
	}
    y+=height;
    g.drawLine(0, y, this->window_width,y);
    }
    /* reference */
    {
    double onebase=distance2pixel(1);
    if(onebase > font_size) {
	int p=interval->start;
	while(p <= interval->end)  {
	     double w= min(font_size,onebase);
	     g.drawText("N",pos2pixel(p)+onebase/2.0-w/2.0, y, w,font_size);
	     p++;
	     }
	y+=font_size;
	g.drawLine(0, y, this->window_width,y);
	y++;
	}
    }

    /** each row */
    double feature_height= std::max(10.0,std::min(30.0,distance2pixel(1)));
    for(size_t y_row=0;y_row< rows.size() && y< this->window_height ;++y_row) {
	double top_row_y = y;
	double midy= top_row_y + feature_height /2.0;
	vector<PairOrReads*>* row=rows[y_row];
	/** all elements on this row */
	for(size_t por_idx=0;por_idx<row->size();++por_idx) {
	    PairOrReads* por = row->at(por_idx);
	    if(this->group_by_pair)
		{
		g.setColor(0.4);
		if(por->first->isPaired() && !por->first->isProperPair()){
		    g.setColor("red");
		    }

		g.drawLine(pos2pixel(por->getStart()), midy, pos2pixel(por->getEnd()+1), midy);
		}
	    // each side of the read pair
	    for(int side=0;side<2;++side) {
		SAMRecord* rec = (side==0?por->first : por->second);
		if(rec==NULL) continue;
		set<double> insertions_x;
		int refPos = rec->getUnclippedStart();
		int readPos = 0;
		Cigar* cigar = rec->getCigar();

		/* rec line */
		g.drawLine(
		    pos2pixel(rec->getUnclippedStart()), midy,
		    pos2pixel(rec->getUnclippedEnd()+1), midy
		    );

		// loop over read
		for(int cigar_index=0;cigar_index < cigar->size();cigar_index++) {
		    CigarElement& ce = cigar->at(cigar_index);
		    int arrow= NO_ARROW;
		    if(rec->isReverseStrand() && cigar_index==0)
			{
			arrow  = ARROW_LEFT;
			}
		    else  if(!rec->isReverseStrand() && cigar_index+1==cigar->size())
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
			    refPos+=ce.len;
			    break;
			    }
			case 'H':
			case 'S':
			case 'X':
			case '=':
			case 'M':
			    {
			    bool bad_read = false;
			    if(rec->isPaired()){
				if(rec->isMateUnmapped()) bad_read=true;
				else if(!rec->isProperPair()) bad_read=true;
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

			    drawArrow(g,
				   fill,
				   stroke,
				   top_row_y,feature_height,refPos,ce.len,
				   arrow
				   );
			    /* draw bases */
			    double one_base=distance2pixel(1);
			    if(ce.op->op!='H' && this->show_base!=display_none && one_base >4) {
				for(int t=0;t< ce.len ;++t) {
				    int basepos = refPos+t;
				    if(basepos< this->interval->start) continue;
				    if(basepos> this->interval->end) break;
				    char base = rec->getBaseAt(readPos+t);
				    g.setColorForBase(base);
				    if(this->show_base==display_readname)
					{
					base = readPos+t >= rec->getReadNameLength()?' ': rec->getReadName()[readPos+t];
					base= rec->isReverseStrand()?tolower(base):toupper(base);
					}
				    double width=std::min(feature_height,one_base);
				    g.drawChar(base, pos2pixel(basepos)+one_base/2.0-width/2.0, top_row_y+2, width, feature_height-4);
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
		    g.setColor("red");
		    g.fillRect(ins_x, top_row_y, 1, feature_height);
		    }

		}


	}
	y+=feature_height+2;
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

	if(tid>=0) {
		int ret;
		bam1_t *b = ::bam_init1();
		int extend=200;
		hts_itr_t *iter = ::sam_itr_queryi(bam->idx, tid,
				std::max(1,interval->start- extend),
				interval->end+extend
				);
		while ((ret = bam_itr_next(bam->fp, iter, b)) >= 0)
			{
			SAMRecord* rec = new SAMRecord(bam->hdr,b,true);
			if(rec->isReadUnmapped() ||
				rec->getReferenceIndex()!=tid ||
					getStart(rec) > interval->end ||
					getEnd(rec)< interval->start)
				{
				delete rec;
				continue;
				}
			if(!group_by_pair)
				{
				all_pairs.push_back(new PairOrReads(this,rec));
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
				cerr << "duplicate read name " << read_name << endl;
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
		all_pairs.push_back(iter->second);
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

void X11Browser::zoomIn() {
    int len  = this->interval->length();
    if(len<=1) return;
    int new_len= std::min(len,(int)(len*0.66)-1);
    if(new_len<=1) return;
    int mid = this->interval->start+len/2;
    this->interval->start = std::max(1,mid-new_len);
    this->interval->end = this->interval->start+new_len;
}

void X11Browser::zoomOut() {
    int len  = this->interval->length();
    int new_len= std::max(len,(int)(len*1.33)+1);
    int mid = this->interval->start+len/2;
    this->interval->start = std::max(1,mid-new_len);
    this->interval->end = this->interval->start+new_len;
}


void X11Browser::goLeft(double factor) {
    int len  = this->interval->length();
    int dlen= std::max(1,(int)(len*factor));
    this->interval->start = std::max(1,this->interval->start - dlen);
    this->interval->end = this->interval->start+len;
    }

void X11Browser::goRight(double factor) {
    int len  = this->interval->length();
    int dlen= std::max(1,(int)(len*factor));
    this->interval->start =  this->interval->start + dlen;
    this->interval->end = this->interval->start+len;
    }

int X11Browser::doWork(int argc,char** argv)
	{
	char* bam_list = NULL;
	char* region_list = NULL;
	int opt;

	if(argc<=1) {
		usage(cerr);
		return EXIT_FAILURE;
		}
	std::string opt_str=  this->build_getopt_str();
	while ((opt = getopt(argc, argv, opt_str.c_str())) != -1) {
		switch (opt) {
		case 'h':
			usage(cout);
			return 0;
		case 'v':
			cout << app_version << endl;
			return 0;
		case 'B':
			bam_list = optarg;
			break;
		case 'R':
			region_list = optarg;
			break;
		case '?':
			cerr << "unknown option -"<< (char)optopt << endl;
			return EXIT_FAILURE;
		default: /* '?' */
			cerr << "unknown option" << endl;
			return EXIT_FAILURE;
		}
	}
	if(optind!=argc) FATAL("Illegal number of arguments.");

	/* BEGIN OPEN BAM FILES ---------------------------------------- */
	if(bam_list == NULL)
	    {
	    FATAL("List of bams is undefined.");
	    }
	else
	    {
	    ifstream bamin(bam_list);
	    if(!bamin.is_open()) FATAL("Cannot open " << bam_list << " "<< strerror(errno));
	    string line;
	    SAMFileFactory samFileFactory;
	    while(getline(bamin,line)) {
		    if(Utils::isBlank(line) || line[0]=='#') continue;
		    SAMFile* bamFile = samFileFactory.open(line.c_str());
		    this->bams.push_back(bamFile);
		    }
	    bamin.close();
	    if(this->bams.empty()) FATAL("List of bams is empty.");
	    }
	/* END OPEN BAM FILES ---------------------------------------- */

	/* BEGIN OPEN BED FILES ---------------------------------------- */
	if(region_list == NULL) {
	    FATAL("List of regions is undefined.");
	    }
	else
	    {
	    string line;
	    BedCodec bedCodec;
	    ifstream bedin(region_list);
	    std::vector<string> tokens;
	    if(!bedin.is_open()) FATAL("Cannot open " << region_list << " "<< strerror(errno));
	    while(getline(bedin,line)) {
	  	if(bedCodec.is_ignore(line)) continue;
	  	Interval* interval = new Interval;
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
	    if(this->intervals.empty())  FATAL("List of regions is empty.");
	    this->interval->assign(this->intervals[0]);
	    }
	/* END OPEN BED FILES ---------------------------------------- */

	//
	this->createWindow();
	//main loop
	XEvent evt;
	bool done=false;
	while(!done) {
		::XNextEvent(this->display, &evt);
		if(evt.type ==  KeyPress)
			{
			if (evt.xkey.keycode == XKeysymToKeycode(this->display, XK_Q) ||
				evt.xkey.keycode == XKeysymToKeycode(this->display, XK_Escape))
				{
				done = true;
				}
			else if (evt.xkey.keycode == XKeysymToKeycode(this->display, XK_Left))
				{
				interval_idx = (interval_idx==0UL?intervals.size()-1:interval_idx-1);
				this->interval->assign(this->intervals.at(interval_idx));
				repaint();
				}
			else if (evt.xkey.keycode == XKeysymToKeycode(this->display, XK_Right))
				{
				interval_idx = (interval_idx+1>=intervals.size()?0:interval_idx+1);
				this->interval->assign(this->intervals.at(interval_idx));
				repaint();
				}
			else if (evt.xkey.keycode == XKeysymToKeycode(this->display, XK_Up))
				{
				bam_index = (bam_index==0UL?bams.size()-1:bam_index-1);
				repaint();
				}
			else if (evt.xkey.keycode == XKeysymToKeycode(this->display, XK_Down))
				{
				bam_index = (bam_index+1>=bams.size()?0:bam_index+1);
				repaint();
				}
			else if (evt.xkey.keycode == XKeysymToKeycode(this->display, XK_plus))
				{
				zoomIn();
				repaint();
				}
			else if (evt.xkey.keycode == XKeysymToKeycode(this->display, XK_minus))
				{
				zoomOut();
				repaint();
				}
			else if (evt.xkey.keycode == XKeysymToKeycode(this->display, XK_A))
				{
				goLeft(0.3);
				repaint();
				}
			else if (evt.xkey.keycode == XKeysymToKeycode(this->display, XK_Z))
				{
				goRight(0.3);
				repaint();
				}
			else if (evt.xkey.keycode == XKeysymToKeycode(this->display, XK_B))
			    {
			    switch(this->show_base)
				{
				case display_base : this->show_base = display_readname;break;
				case display_readname : this->show_base = display_none;break;
				default: this->show_base = display_base; break;
				}
			    repaint();
			    }
			}
		else if(evt.type ==   Expose)
			{
			resized();
			}
		}//end while
	disposeWindow();
	return EXIT_SUCCESS;
	}

int main_browser(int argc,char** argv) {
    X11Browser app;
    return app.doWork(argc,argv);
    }
