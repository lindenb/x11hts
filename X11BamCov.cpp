/*
 * X11BamCov.cpp
 *
 *  Created on: Mar 25, 2019
 *      Author: lindenb
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
#include <limits.h>
#include <unistd.h>
#include <getopt.h>

#include <htslib/sam.h>
#include <htslib/faidx.h>
#include <htslib/kstring.h>
#include <htslib/khash_str2int.h>


using namespace std;

class BamW;

class ChromStartEnd
	{
public:
	std::string chrom;
	int start;
	int end;
	ChromStartEnd(std::string line) {
		std::string::size_type p1 = line.find('\t');
		if(p1==string::npos) {
			ostringstream os;
			os << "cannot find first tab in " << line ;
			throw invalid_argument(os.str());
			}
		std::string::size_type p2 = line.find('\t',p1+1);
		if(p2==string::npos) {
			ostringstream os;
			os << "cannot find second tab in " << line ;
			throw invalid_argument(os.str());
			}
		std::string::size_type p3 = line.find('\t',p2+1);
		if(p3==string::npos) p3=line.length();
		this->chrom.assign(line.substr(0,p1));
		this->start= 1 + atoi(line.substr(p1+1,p2-p1).c_str());
		this->end= atoi(line.substr(p2+1,p3).c_str());
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

	X11BamCov();
	~X11BamCov();
	int doWork(int argc,char** argv);
	void repaint();
	void paint();
	void resized();

	static int callback(void *data, bam1_t *b);
	};


class BamW
	{
public:
	X11BamCov* owner;
	std::string filename;
	std::vector<float> coverage;
	samFile *fp;
	bam_hdr_t *hdr;  // the file header
	hts_idx_t *idx = NULL;
	BamW(X11BamCov* owner,std::string fn);
	~BamW();
	};

BamW::BamW(X11BamCov* owner,std::string fn):owner(owner),filename(fn) {
	
	fp = ::hts_open(fn.c_str(), "r");
	if(fp==NULL) {
		cerr << "Cannot open " << fn << "." << endl;
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
	}

BamW::~BamW() {
	::hts_idx_destroy(idx);
	::bam_hdr_destroy(hdr);
	::hts_close(fp);
	}


X11BamCov::X11BamCov() {
	region_idx = 0UL;
	window_width = 0;
	window_height = 0;
}
X11BamCov::~X11BamCov() {
for(auto iter:bams) {
	delete iter;
	}
for(auto iter:regions) {
	delete iter;
	}
}

void X11BamCov::paint() {
}


static int X11BamCov::callback(void *data, bam1_t *b)
{
    phaseg_t *g = (phaseg_t*)data;
    int ret;
    while (1)
    {
        ret = sam_read1(g->fp, g->fp_hdr, b);
        if (ret < 0) break;
        if ( b->core.flag & (BAM_FUNMAP | BAM_FSECONDARY | BAM_FQCFAIL | BAM_FDUP) ) continue;
        if ( g->pre ) {
            if (g->n == g->m) {
                g->m = g->m? g->m<<1 : 16;
                g->b = realloc(g->b, g->m * sizeof(bam1_t*));
            }
            g->b[g->n++] = bam_dup1(b);
        }
        break;
    }
    return ret;
}


void X11BamCov::repaint() {

int max_depth = 0;

ChromStartEnd* rgn = this->regions[this->region_idx];


//reload data for each bam
for(auto bam: this->bams) {
	bam->coverage.clear();
	if(this->window_width<2) continue;
	bam->coverage.resize(this->window_width,0);
	int tid = ::bam_name2id(bam->hdr, rgn->chrom.c_str());
	if(tid<0) {
		cerr << "[WARN] No chromosome " << rgn->chrom << " in "<< bam ->filename << endl;
		continue;
		}
	hts_itr_t *iter = ::sam_itr_queryi(bam->idx, tid,rgn->start,rgn->end);
	bam_plp_t iterp = bam_plp_init(X11BamCov::callback, bam);
	bam_pileup1_t *plp;
	int tid2;
	int pos;
	int n;
	while ((plp = bam_plp_auto(iterp, &tid2, &pos, &n)) != 0) {
		}

	 bam_plp_destroy(iterp);
	
	::hts_itr_destroy(iter);
	}

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

	int opt;
	while ((opt = ::getopt(argc, argv, "B:R:")) != -1) {
		switch (opt) {
		case 'B':
			bam_list = optarg;
			break;
		case 'R':
			region_list = optarg;
			break;
		default: /* '?' */
			cerr << "unknown option" << endl;
			return EXIT_FAILURE;
		}
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
		this->regions.push_back(rgn);
		}
	regionin.close();
	if(this->regions.empty()) {
		cerr << "[FAILURE] List of regions is empty." << endl;
		return EXIT_FAILURE;
		}
	//
	this->display = ::XOpenDisplay(NULL);
	if (this->display == NULL) {
	   cerr<<  "[FAILURE] Cannot open display." << endl;
	   return(EXIT_FAILURE);
	   }
	 this->screen_number = DefaultScreen(display);
	 screen = ::XScreenOfDisplay(this->display, this->screen_number);
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
			}
		else if(evt.type ==   Expose)
			{
			resized();
			break;
			}
		}//end while

	::XCloseDisplay(display);
	display=NULL;
	return 0;
	}

int main_cnv(int argc,char** argv) {
	X11BamCov app;
	return app.doWork(argc,argv);
	}
