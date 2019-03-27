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
#include <X11/Xlib.h>
#include <X11/keysym.h>
#include <X11/Xutil.h>
#include <iostream>
#include <vector>
#include <iostream>
#include <stdexcept>
#include <fstream>
#include <sstream>
#include <cmath>
#include <limits.h>
#include <unistd.h>
#include <getopt.h>
#include <cerrno>

#include <htslib/sam.h>
#include <htslib/faidx.h>
#include <htslib/kstring.h>
#include <htslib/khash_str2int.h>

#include "Palette.hh"
#include "Hershey.hh"

using namespace std;

class BamW;

typedef short pixel_t;

static int parseInt(const char* s) {
char* p2;
errno=0;
long int i = strtol(s,&p2,10);
if(*p2!=0 || errno!=0) {
	cerr << "bad number " << s << endl;
	exit(EXIT_FAILURE);
	}
return i;
}

static string niceInt(int i) {
	ostringstream os;
	os << i;
	string s1=os.str();
	string s2;
	for(size_t k=0; k < s1.length();k++)
		{
		if(k>0 && k%3==0) s2=","+s2;
		s2= s1[(s1.length()-1)-k]+s2;
		}
	return s2;
	}

static bool starts_with(std::string s1,const char* s2) {
	size_t len2=strlen(s2);
	if(len2> s1.size()) return false;
	return s1.compare(0,len2, s2) == 0;
	}

class ChromStartEnd
	{
public:
	std::string chrom;
	int start;
	int end;
	std::string label;
	int original_start;
	int original_end;
	ChromStartEnd(std::string line):label("") {
		std::string::size_type p1 = line.find('\t');
		if(p1==string::npos) {
			std::string::size_type p1 = line.find(':');
			if(p1==string::npos) {
				ostringstream os;
				os << "cannot find first tab or colon in " << line ;
				throw invalid_argument(os.str());
				}
			std::string::size_type p2 = line.find('-',p1+1);
			if(p2==string::npos) {
				ostringstream os;
				os << "cannot find hyphen in " << line ;
				throw invalid_argument(os.str());
				}
			this->chrom.assign(line.substr(0,p1));
			this->start= parseInt(line.substr(p1+1,p2-(p1+1)).c_str());
			this->end= parseInt(line.substr(p2+1).c_str());
			}
		else
			{
			std::string::size_type p2 = line.find('\t',p1+1);
			if(p2==string::npos) {
				ostringstream os;
				os << "cannot find second tab in " << line ;
				throw invalid_argument(os.str());
				}
			std::string::size_type p3 = line.find('\t',p2+1);
			if(p3==string::npos) p3=line.length();
			this->chrom.assign(line.substr(0,p1));
			this->start= 1 + parseInt(line.substr(p1+1,p2-(p1+1)).c_str());
			this->end= parseInt(line.substr(p2+1,p3).c_str());
			}
		if(this->start >= this->end) {
			cerr << "[FATAL] Empty/negative interval " << line << endl;
			exit(EXIT_FAILURE);			
			}
		this->original_start = this->start;
		this->original_end = this->end;
		}
	int length() {
		return 1+ (this->end - this->start);		
		}

	};


class X11BamCov
	{
public:
	Display *display;
	Screen *screen;
	int screen_number;
	Window window;
	std::vector<BamW*> bams;
	std::vector<ChromStartEnd*> regions;
	size_t region_idx;
	int window_width;
	int window_height;
	Hershey hershey;
	float extend_factor;
	int num_columns;
	Palette* palette;
	int cap_depth;

	X11BamCov();
	~X11BamCov();
	int doWork(int argc,char** argv);
	void repaint();
	void paint();
	void resized();

	};


class BamW
	{
	public:
		X11BamCov* owner;
		std::string filename;
		std::string sample;
		std::vector<float> coverage;
		samFile *fp;
		bam_hdr_t *hdr;  // the file header
		hts_idx_t *idx = NULL;
		bool bad_flag;
		double max_depth;
		XRectangle bounds;
		
		BamW(X11BamCov* owner,std::string fn);
		~BamW();
	};

BamW::BamW(X11BamCov* owner,std::string fn):owner(owner),filename(fn),sample(fn) {
	
	fp = ::hts_open(fn.c_str(), "r");
	if(fp==NULL) {
		cerr << "Cannot open " << fn << ". " << ::strerror(errno) << endl;
		exit(EXIT_FAILURE);
		}
	hdr = sam_hdr_read(fp); 
	if (hdr == NULL) {
            cerr << "Cannot open header for " << fn << "." << endl;
            exit(EXIT_FAILURE);
            }

	idx = sam_index_load(this->fp,fn.c_str());
	if(idx==NULL) {
		cerr << "Cannot open index for " << fn << "." << endl;
		exit(EXIT_FAILURE);
		}


	if(hdr->text!=NULL)
		{
		
		std::istringstream iss(hdr->text);
		std::string line;
       		 while(std::getline(iss, line, '\n'))
			{
			if(!starts_with(line,"@RG\t")) continue;
			string::size_type  p = line.find("\tSM:");
			if(p==string::npos) continue;
			p+=4;
			string::size_type  p2 = line.find("\t");
			if(p2==string::npos) p2=line.size();
			sample = line.substr(p,p2);
			break;
			}
		}

	}

BamW::~BamW() {
	::hts_idx_destroy(idx);
	::bam_hdr_destroy(hdr);
	::hts_close(fp);
	}


X11BamCov::X11BamCov():palette(0) {
	region_idx = 0UL;
	window_width = 0;
	window_height = 0;
	num_columns = 1 ;
	extend_factor = 0.0f;
	cap_depth = -1;
	}


X11BamCov::~X11BamCov() {
	for(auto iter:bams) {
		delete iter;
		}
	for(auto iter:regions) {
		delete iter;
		}
	if(palette!=0) delete palette;
	}
#define MARGIN_TOP 20
void X11BamCov::paint() {
GC gc = ::XCreateGC(this->display, this->window, 0, 0);
XSetForeground(this->display, gc, WhitePixel(this->display, this->screen_number));
::XFillRectangle(this->display,this->window, gc,0,0,this->window_width,this->window_height);

double max_depth = 1.0;


for(auto bam: this->bams) {
	max_depth = std::max(max_depth,bam->max_depth);
	}
for(auto bam: this->bams) {
	bam->max_depth = max_depth;
	}
XSetForeground(this->display, gc, BlackPixel(this->display, this->screen_number));

ChromStartEnd* rgn = this->regions[this->region_idx];
string win_title;
{
	ostringstream os;
	os << rgn->chrom << ":" << niceInt(rgn->start) << "-" << niceInt(rgn->end);
	win_title.assign(os.str());
	XStoreName(this->display,this->window,win_title.c_str());
}

	{
	ostringstream os;
	os << win_title
			<< " maxDepth:"<< max_depth << " length: "<< niceInt(rgn->length()) << rgn->label
			<< " (" << niceInt(this->region_idx+1) << "/"
			<< niceInt((int)this->regions.size()) << ")"
			;
	string title= os.str();
	int title_width= title.size()*12;
	hershey.paint(this->display,this->window, gc,title.c_str(),
			this->window_width/2 - title_width/2,
			1,
			title_width,
			MARGIN_TOP-2
			);
	}

for(auto bam: this->bams) {

	int ruledy=1.0;
	if(bam->max_depth>100) {
		ruledy=100;
	} if(bam->max_depth>50) {
		ruledy=10;
	} else if(bam->max_depth>10) {
		ruledy=5;
	}else
	{
		ruledy=1;
	}

   vector<XPoint> points;
   XPoint pt1={(pixel_t)bam->bounds.x,(pixel_t)(bam->bounds.y+bam->bounds.height)};
   points.push_back(pt1);
   for(size_t i=0;i< bam->coverage.size();i++)
   		{
   		double h = (bam->coverage[i]/bam->max_depth)*bam->bounds.height;
   		XPoint pt;
   		pt.x = (pixel_t)(bam->bounds.x+i);
   		pt.y = (pixel_t)(bam->bounds.y+bam->bounds.height - h);
   		points.push_back(pt);
   		}
   XPoint pt2={
		(pixel_t)(bam->bounds.width+bam->bounds.x),
		(pixel_t)(bam->bounds.y+bam->bounds.height)
		};
   points.push_back(pt2);
   points.push_back(pt1);

  if(rgn->start!=rgn->original_start || rgn->end!=rgn->original_end) {
  	pixel_t x1 = (pixel_t)(bam->bounds.x + ((rgn->original_start-rgn->start)/(double)rgn->length())*bam->bounds.width);
	pixel_t x2 = (pixel_t)(bam->bounds.x + ((rgn->original_end-rgn->start)/(double)rgn->length())*bam->bounds.width);
	XSetForeground(this->display, gc,palette->gray(0.9).pixel);
	::XFillRectangle(this->display,this->window, gc,x1,bam->bounds.y,(x2-x1),bam->bounds.height);
  }
  // print ruler
  double curr_depth = ruledy;
  while(curr_depth <= bam->max_depth) {
   	  double y =  bam->bounds.y + bam->bounds.height- ((curr_depth/bam->max_depth) * bam->bounds.height);
	  if(y <  bam->bounds.y) break;
	  XSetForeground(this->display, gc,palette->gray(0.8).pixel);
	  XDrawLine(this->display, this->window, gc, (int)bam->bounds.x, (int)y,(int)(bam->bounds.x+bam->bounds.width), (int)y);
	  curr_depth+=ruledy;
  	  }


   XSetForeground(this->display, gc,palette->dark_slate_gray.pixel);
   ::XFillPolygon(this->display,this->window, gc, &points[0], (int)points.size(), Complex,CoordModeOrigin);

   XSetForeground(this->display, gc, BlackPixel(this->display, this->screen_number));
   hershey.paint(this->display,this->window, gc,bam->sample.c_str(),
		bam->bounds.x,
		bam->bounds.y+1,
		std::min((int)bam->bounds.width,12*(int)bam->sample.size()),
		20
		);

   curr_depth = ruledy;
   while(curr_depth <= bam->max_depth) {
   	  double y =  bam->bounds.y + bam->bounds.height- ((curr_depth/bam->max_depth) * bam->bounds.height);
   	  if(y <  bam->bounds.y) break;
   	  XSetForeground(this->display, gc,palette->gray(0.8).pixel);
   	  XDrawLine(this->display, this->window, gc, (int)bam->bounds.x, (int)y,(int)(bam->bounds.x+bam->bounds.width), (int)y);
   	  XSetForeground(this->display, gc,palette->gray(0.1).pixel);
   	  char tmp[20];
   	  sprintf(tmp,"%d",(int)curr_depth);
   	  if(y-7 > bam->bounds.y) {
		  hershey.paint(this->display,this->window,gc,
				tmp,
				bam->bounds.x+1,
				(int)y-7,
				(int)7*strlen(tmp),
				7
				);
		  }
   	  curr_depth+=ruledy;
     }

   ::XDrawRectangle(this->display,this->window, gc,
		bam->bounds.x,
		bam->bounds.y,
		bam->bounds.width,
		bam->bounds.height
		);
   }
}




void X11BamCov::repaint() {

int ret = 0;
ChromStartEnd* rgn = this->regions[this->region_idx];
vector<int> coverage;
coverage.resize(rgn->length(),0);

int curr_x=0;
int curr_y=0;
int n_rows = (int) ceil(this->bams.size()/(double)this->num_columns);
if(n_rows<0) n_rows=1;


int rect_w = (this->window_width /  this->num_columns);
if(rect_w< 1) return;
int rect_h = ((this->window_height-MARGIN_TOP) /n_rows);
if(rect_h< 1) return;
vector<int> counts;

bam1_t *b = ::bam_init1();

//reload data for each bam
for(auto bam: this->bams) {
	counts.clear();



	bam->bad_flag = false;
	bam->max_depth = 1.0;
	bam->bounds.y = MARGIN_TOP + curr_y*rect_h;
	bam->bounds.x = curr_x*rect_w;
	bam->bounds.width = rect_w;
	bam->bounds.height = rect_h;
	curr_y++;
	if(curr_y>=n_rows)
		{
		curr_y=0;
		curr_x++;
		}
	

	bam->coverage.clear();
	
	bam->coverage.resize(bam->bounds.width,0);
	std::fill(coverage.begin(),coverage.end(),0);
	
	

	int tid = ::bam_name2id(bam->hdr, rgn->chrom.c_str());
	if(tid<0 && starts_with(rgn->chrom,"chr"))
		{
		string ctg2 = rgn->chrom.substr(3);
		tid = ::bam_name2id(bam->hdr, ctg2.c_str());
		}
	if(tid<0 && !starts_with(rgn->chrom,"chr"))
		{
		string ctg2 = "chr";
		ctg2.append(rgn->chrom);
		tid = ::bam_name2id(bam->hdr, ctg2.c_str());
		}

	if(tid<0) {
		bam->bad_flag = true;
		cerr << "[WARN] No chromosome " << rgn->chrom << " in "<< bam ->filename << endl;
		continue;
		}
		
	hts_itr_t *iter = ::sam_itr_queryi(bam->idx, tid,rgn->start,rgn->end);
	while ((ret = bam_itr_next(bam->fp, iter, b)) >= 0)
		{
		const bam1_core_t *c = &b->core;
		if ( c->flag & (BAM_FUNMAP | BAM_FSECONDARY | BAM_FQCFAIL | BAM_FDUP) ) continue;
		
		uint32_t *cigar = bam_get_cigar(b);
		if(cigar==NULL) continue;
		
		int ref1 = c->pos + 1;
		
		for (unsigned int icig=0; icig< c->n_cigar && ref1 < rgn->end; icig++)
	    		{
			int op  = bam_cigar_opchr(cigar[icig]);
			int len = bam_cigar_oplen(cigar[icig]);
			    
		    	switch(op)
		    		{
		    		case 'P': break;
		    		case 'I': break;
		    		case 'D': case 'N' : ref1+=len; break;
		    		case 'S': case 'H':break;
		    		case 'M': case '=' : case 'X':
		    			{
		    			for(int x=0;x< len && ref1 < rgn->end ;++x) {
		    				int idx1 = ref1 - rgn->start;
						ref1++;
		    				if(idx1< 0 || idx1 >= (int)coverage.size()) continue;
						coverage[idx1]++;
						bam->max_depth = std::max(bam->max_depth,(double)coverage[idx1]);
		    				}
		    			break;
		    			}
		    		default: cerr << "boum ??" <<(char) op<<" " <<(char) BAM_CMATCH << endl;break;
		    		}
			}
		}
	::hts_itr_destroy(iter);
	if(this->cap_depth>0) bam->max_depth=std::min(bam->max_depth,(double)this->cap_depth);

	int smooth=5;
	if(smooth>0) {
		vector<int> smoothed;
		smoothed.resize(coverage.size(),0);	
		for(int i=0;i< (int)coverage. size();i++)
			{
			double total=0;
			int count=0;
			for(int j=std::max(0,i-smooth);j<i+smooth;++j)
				{
				if(j<0 || j>= (int)coverage.size()) continue;
				total +=  coverage[j];
				count++;
				}
			if(count==0) continue;
			smoothed[i]=(int)(total/count);
			}
		for(int i=0;i< (int)coverage. size();i++)
			{
			coverage[i]=smoothed[i];
			}
		}

	for(int i=0;i< (int)bam->coverage.size();i++)
		{
		int g1 = (i/(double)bam->coverage.size())*coverage.size();
		int g2 = ((i+1)/(double)bam->coverage.size())*coverage.size();
		double total=0;
		int count=0;
		for(int x=g1;x<=g2 && x < (int)coverage.size();++x)
			{
			total+= coverage[x];
			count++;
			}
		if(count==0) continue;
		bam->coverage[i]= total/count;
		if(this->cap_depth>0) bam->coverage[i] = std::min(bam->coverage[i],(float)this->cap_depth);
		}
	}
::bam_destroy1(b);

paint();
}

void X11BamCov::resized() {
	//int x,y,wr;
	//unsigned int w,h,bw, d;
	 XWindowAttributes att;
	::XGetWindowAttributes(display, window, &att);
	if(att.width!=this->window_width || att.height!=this->window_height) {
		this->window_width = att.width;
		this->window_height =  att.height;
		repaint();
		}
	}



int X11BamCov::doWork(int argc,char** argv) {
	char* bam_list = NULL;
	char* region_list = NULL;
	char *file_out = NULL;
	int opt;
	while ((opt = getopt(argc, argv, "B:R:f:D:o:vh")) != -1) {
		switch (opt) {
		case 'h':
			cout << "cnv" << endl;
			cout << "Motivation:\n  Displays Bam coverage" << endl;
			cout << "Keys:\n";
			cout << "  'S' save current segment in output file\n";
			cout << "  '<-' previous interval\n";
			cout << "  '->' next interval\n";
			cout << "  'R'/'T' change column number\n";
			cout << "  'Q'/'Esc' exit\n";
			cout << "Options:\n";
			cout << "  -h print help and exit\n";
			cout << "  -v print version and exit\n";
			cout << "  -o (FILE) save BED segment in that bed file (use key 'S')\n";
			cout << "  -D (int) cap depth to that value\n";
			cout << "  -B (FILE) list of path to indexed bam files\n";
			cout << "  -R (FILE) bed file of regions of interest\n";
			cout << "  -f (float) extend the regions by this factor\n";
			return 0;
		case 'v':
			cout << "cnv\nAuthor: Pierre Lindenbaum PhD.\nCompilation: " << __DATE__ << endl;
			return 0;
		case 'o':
			file_out = optarg;
			break;
		case 'D':
			this->cap_depth = atoi(optarg);
			break;
		case 'B':
			bam_list = optarg;
			break;
		case 'R':
			region_list = optarg;
			break;
		case 'f': 
			this->extend_factor = atof(optarg);
			break;
		case '?':
			cerr << "unknown option -"<< (char)optopt << endl;
			return EXIT_FAILURE;
		default: /* '?' */
			cerr << "unknown option" << endl;
			return EXIT_FAILURE;
		}
	}
	if(optind!=argc) {
		cerr << "Illegal number of arguments." << endl;
		return EXIT_FAILURE;
		}
	//
	if(bam_list == NULL) {
		cerr << "List of bams is undefined." << endl;
		return EXIT_FAILURE;
		}
	ifstream bamin(bam_list);
	if(!bamin.is_open()) {
		cerr << "Cannot open " << bam_list << endl;
		return EXIT_FAILURE;
		}
	string line;
	while(getline(bamin,line)) {
		if(line.empty() || line[0]=='#') continue;
		BamW* bamFile	 = new BamW(this,line);
		this->bams.push_back(bamFile);
		}
	bamin.close();
	if(this->bams.empty()) {
		cerr << "List of bams is empty." << endl;
		return EXIT_FAILURE;
		}
	this->num_columns = (int)std::ceil(::sqrt(this->bams.size()));
	if( this->num_columns <= 0 ) this->num_columns = 1;
	//cerr << "[DEBUG]ncols " << num_columns << endl;

	//
	if(region_list == NULL) {
		cerr << "List of regions is undefined." << endl;
		return EXIT_FAILURE;
		}
	ifstream regionin(region_list);
	if(!regionin.is_open()) {
		cerr << "Cannot open " << region_list << endl;
		return EXIT_FAILURE;
		}
	while(getline(regionin,line)) {
		if(line.empty() || line[0]=='#') continue;
		ChromStartEnd* rgn	 = new ChromStartEnd(line);
		if(this->extend_factor != 0.0)
			{
			int L = rgn->length();
			int L2 = (int)(L*(1.0+extend_factor));
			int mid = rgn->start + L/2;
			rgn->start = std::max(1,mid - L2);
			rgn->end = mid + L2;
			//cerr << "[DEBUG]" << rgn->start << "-" << rgn->end  << endl;
			}

		this->regions.push_back(rgn);
		}
	regionin.close();
	if(this->regions.empty()) {
		cerr << "[FAILURE] List of regions is empty." << endl;
		return EXIT_FAILURE;
		}
	//
	FILE* saveOut=NULL;
	if(file_out!=NULL)
		{
		saveOut = fopen(file_out,"w");
		if(saveOut==NULL) {
			cerr << "Cannot open " << file_out << endl;
			return EXIT_FAILURE;			
			}
		}

	//
	this->display = ::XOpenDisplay(NULL);
	if (this->display == NULL) {
	   cerr<<  "[FAILURE] Cannot open display." << endl;
	   return(EXIT_FAILURE);
	   }

	

	 this->screen_number = DefaultScreen(this->display);

	 this->palette = new Palette(this->display,  this->screen_number);

	 this->screen = ::XScreenOfDisplay(this->display, this->screen_number);
	 this->window = ::XCreateSimpleWindow(
			 display,
			 RootWindow(this->display,  this->screen_number),
			 150, 150,
			 screen->width-300,
			 screen->height-300,
			 1,
	         BlackPixel(display,  this->screen_number),
			 WhitePixel(display,  this->screen_number)
			 );

	::XSelectInput(display, window, ExposureMask | KeyPressMask);
	::XMapWindow(display, window);
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
				region_idx = (region_idx==0UL?regions.size()-1:region_idx-1);
				repaint();
				}
			else if (evt.xkey.keycode == XKeysymToKeycode(this->display, XK_Right))
				{
				region_idx = (region_idx+1>=regions.size()?0:region_idx+1);
				repaint();
				}
			else if (evt.xkey.keycode == XKeysymToKeycode(this->display, XK_S) && saveOut!=NULL)
				{
				ChromStartEnd* rgn = this->regions[this->region_idx];
				fprintf(saveOut,"%s\t%d\t%d\n",
					rgn->chrom.c_str(),
					rgn->original_start-1,
					rgn->original_end
					);
				cerr << "[INFO] SAVED" << endl;
				}
			else if (evt.xkey.keycode == XKeysymToKeycode(this->display, XK_R) && num_columns>1)
				{
				num_columns--;
				repaint();
				}
			else if (evt.xkey.keycode == XKeysymToKeycode(this->display, XK_T) && num_columns+1<= (int)this->bams.size())
				{
				num_columns++;
				repaint();
				}
			}
		else if(evt.type ==   Expose)
			{
			resized();
			}
		}//end while

	::XCloseDisplay(display);
	display=NULL;
	if(saveOut!=NULL)
		{
		fclose(saveOut);
		}
	return 0;
	}

int main_cnv(int argc,char** argv) {
	X11BamCov app;
	return app.doWork(argc,argv);
	}
