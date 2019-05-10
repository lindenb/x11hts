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
#ifndef WITHOUT_X11
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
#include <memory>

#include <htslib/sam.h>
#include <htslib/faidx.h>
#include <htslib/kstring.h>
#include <htslib/khash_str2int.h>

#include "X11Launcher.hh"
#include "Utils.hh"
#include "Interval.hh"
#include "Locatable.hh"
#include "SAMFile.hh"
#include "Utils.hh"
#include "Hershey.hh"
#include "version.hh"
#include "macros.hh"
#include "Graphics.hh"
#include "BedLine.hh"

using namespace std;

class BamW;

#define THROW_INVALID_ARG(a) do {\
	cerr << a << endl;\
	ostringstream _os; _os << "[ERROR]" << __FILE__ <<":" << __LINE__ << ": " << a ; \
	throw invalid_argument(_os.str());\
	} while(0)

typedef short pixel_t;




/** an interval */
class ChromStartEnd : public Interval
	{
	public:
	    /** remaining element in the bed file , 4th column */
	    std::string label;
	    /** constructor: accept bed or interval */
	    ChromStartEnd(const char* chrom,int start,int end):Interval(chrom,start,end) {
		}
	    virtual ~ChromStartEnd() {
		}
	};





class X11BamCov: public X11Launcher
	{
    public:
	    std::vector<BamW*> bams;
	    std::vector<ChromStartEnd*> regions;
	    int smooth_factor;
	    size_t region_idx;
	    float extend_factor;
	    ChromStartEnd* interval;
	    int num_columns;
	    int cap_depth;
	    bool show_sample_name;
	    X11BamCov();
	    virtual ~X11BamCov();
	    virtual int doWork(int argc,char** argv);
	    virtual void repaint();
	    virtual void paint();
	    virtual void paint(Graphics& g);
	};


class X11BamCovFileFactory: public SAMFileFactory
	{
public:
	X11BamCov* owner;
	X11BamCovFileFactory(X11BamCov* owner);
	virtual ~X11BamCovFileFactory();
	virtual SAMFile* createInstance();
	};

class BamW : public SAMFile
	{
	public:
		double max_depth;
		std::string sample;
		std::vector<float> coverage;
		bool bad_flag;
		X11BamCov* owner;
		XRectangle bounds;
		
		BamW(X11BamCov* owner);
		~BamW();
	};

X11BamCovFileFactory::X11BamCovFileFactory(X11BamCov* owner):owner(owner){
}
X11BamCovFileFactory::~X11BamCovFileFactory(){

}

SAMFile* X11BamCovFileFactory::createInstance() {
	BamW* instance= new BamW(this->owner);
	return instance;
}

BamW::BamW(X11BamCov* owner):max_depth(0),bad_flag(false),owner(owner) {
	}

BamW::~BamW() {
	}


X11BamCov::X11BamCov():
	smooth_factor(20),
	interval(0)
	{
	region_idx = 0UL;
	window_width = 0;
	window_height = 0;
	num_columns = 1 ;
	extend_factor = 0.0f;
	cap_depth = -1;
	app_name.assign("cnv");
	app_desc.assign("Displays Bam coverage");

	 Option* opt= new Option('o',true,"Export directory; Where to save screenshots.");
	 opt->arg("directory");
	 options.push_back(opt);

	}


X11BamCov::~X11BamCov() {
    if(interval!=0) delete interval;
    for(auto iter:bams) {
	    delete iter;
	    }
    for(auto iter:regions) {
	    delete iter;
	    }
    }

#define MARGIN_TOP 20
void X11BamCov::paint() {
    X11Graphics g(this->display, this->window);
    paint(g);
}


void X11BamCov::paint(Graphics& g) {

g.setColorName("white");
g.fillRect(0,0,this->window_width,this->window_height);

double max_depth = 1.0;


for(auto bam: this->bams) {
	max_depth = std::max(max_depth,bam->max_depth);
	}
for(auto bam: this->bams) {
	bam->max_depth = max_depth;
	}
g.setColorName("black");


ChromStartEnd* rgn = this->interval;
string win_title;
{
	ostringstream os;
	os << rgn->getContig() << ":" << Utils::niceInt(rgn->getStart()) << "-" << Utils::niceInt(rgn->getEnd());
	win_title.assign(os.str());
	XStoreName(this->display,this->window,win_title.c_str());
}

	{
	ostringstream os;
	os << win_title
			<< " maxDepth:"<< max_depth << " length: "<< Utils::niceInt(rgn->getLengthOnReference())
			<< " \"" << rgn->label << "\" "
			<< " (" << Utils::niceInt(this->region_idx+1) << "/"
			<< Utils::niceInt((int)this->regions.size()) << ")"
			;
	string title= os.str();
	int title_width= title.size()*12;
	g.drawText(
			title.c_str(),
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

   double* array_x=new double[2+bam->coverage.size()];
   double* array_y=new double[2+bam->coverage.size()];
   array_x[0] = (bam->bounds.width+bam->bounds.x);
   array_y[0] = (bam->bounds.y+bam->bounds.height);

   array_x[1] = (bam->bounds.x);
   array_y[1] = (bam->bounds.y+bam->bounds.height);

   for(size_t i=0;i< bam->coverage.size();i++)
   		{
   		double h = (bam->coverage[i]/bam->max_depth)*bam->bounds.height;
   		array_x[2+i] = (bam->bounds.x+i);
   		array_y[2+i] = (bam->bounds.y+bam->bounds.height - h);
   		}

   ChromStartEnd* original = this->interval;

  if(rgn->getStart()!=original->getStart() || rgn->getEnd()!= original->getEnd()) {
  	pixel_t x1 = (pixel_t)(bam->bounds.x + ((original->getStart()-rgn->getStart())/(double)rgn->getLengthOnReference())*bam->bounds.width);
	pixel_t x2 = (pixel_t)(bam->bounds.x + ((original->getEnd()-rgn->getStart())/(double)rgn->getLengthOnReference())*bam->bounds.width);
	g.setGray(0.9);
	g.fillRect(x1,bam->bounds.y,(x2-x1),bam->bounds.height);
      }
  // print ruler
  double curr_depth = ruledy;
  while(curr_depth <= bam->max_depth) {
   	  double y =  bam->bounds.y + bam->bounds.height- ((curr_depth/bam->max_depth) * bam->bounds.height);
	  if(y <  bam->bounds.y) break;
	  g.setGray(0.8);
	  g.drawLine(
		  bam->bounds.x, y,
		  (bam->bounds.x+bam->bounds.width),
		  y);
	  curr_depth+=ruledy;
  	  }

   g.setColor(47, 79, 79);
   g.fillPolygon(2+bam->coverage.size(),array_x,array_y);
   delete[] array_x;
   delete[] array_y;
 
  


  
   curr_depth = ruledy;
   while(curr_depth <= bam->max_depth) {
   	  double y =  bam->bounds.y + bam->bounds.height- ((curr_depth/bam->max_depth) * bam->bounds.height);
   	  if(y <  bam->bounds.y) break;
   	  //g.setGray(0.8);
	  //XSetFunction(this->display, gc, GXxor);
   	  g.drawLine( bam->bounds.x,y,(bam->bounds.x+bam->bounds.width),y);
          //XSetFunction(this->display, gc, GXcopy);

   	  g.setGray(0.5);
   	  char tmp[20];
   	  sprintf(tmp,"%d",(int)curr_depth);
   	  if(y-7 > bam->bounds.y) {
   		  g.drawText(tmp,
   			 bam->bounds.x+1,
			(int)y-7,
			(int)7*strlen(tmp),
			7);
		  }
   	  curr_depth+=ruledy;
	 }
  

  if(this->show_sample_name) {
        g.setGray(0.1);
        g.drawText(
        	bam->sample.c_str(),
		bam->bounds.x,
		bam->bounds.y+1,
		std::min((int)bam->bounds.width,12*(int)bam->sample.size()),
		std::min(20,(int)(bam->bounds.height/10))
		);
	}
    g.setGray(0.0);
    g.drawRect( bam->bounds.x,
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
coverage.resize(rgn->getLengthOnReference(),0);

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
	curr_x++;
	if(curr_x>=this->num_columns)
		{
		curr_x=0;
		curr_y++;
		}
	

	bam->coverage.clear();
	
	bam->coverage.resize(bam->bounds.width,0);
	std::fill(coverage.begin(),coverage.end(),0);
	
	

	int tid = bam->contigToTid(rgn->getContig());

	if(tid<0) {
		bam->bad_flag = true;
		cerr << "[WARN] No chromosome " << rgn->getContig() << " in "<< bam ->filename << endl;
		continue;
		}
		
	hts_itr_t *iter = ::sam_itr_queryi(bam->idx, tid,rgn->getStart(),rgn->getEnd());
	while ((ret = bam_itr_next(bam->fp, iter, b)) >= 0)
		{
		const bam1_core_t *c = &b->core;
		if ( c->flag & (BAM_FUNMAP | BAM_FSECONDARY | BAM_FQCFAIL | BAM_FDUP) ) continue;
		
		uint32_t *cigar = bam_get_cigar(b);
		if(cigar==NULL) continue;
		
		int ref1 = c->pos + 1;
		
		for (unsigned int icig=0; icig< c->n_cigar && ref1 < rgn->getEnd(); icig++)
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
		    			for(int x=0;x< len && ref1 < rgn->getEnd() ;++x) {
		    				int idx1 = ref1 - rgn->getStart();
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

	int smooth=0;
	if(smooth_factor>1) smooth = (int)(coverage.size()/(double)this->smooth_factor);
	if(smooth>0) {
		vector<int> smoothed;
		smoothed.resize(coverage.size(),0);
		std::fill(smoothed.begin(),smoothed.end(),0);
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





int X11BamCov::doWork(int argc,char** argv) {
	int id_generator = 0;
	const KeyAction* actionQuit = createKeyAction(XKEY_STR(XK_Q), "Exit.");
	const KeyAction* actionEscape = createKeyAction(XKEY_STR(XK_Escape), "Exit.");
	const KeyAction* actionNextInterval = createKeyAction(XKEY_STR(XK_L), "Move to next interval.");
	const KeyAction* actionPrevInterval = createKeyAction(XKEY_STR(XK_H), "Move to previous interval.");
	const KeyAction* actionExportPS = createKeyAction(XKEY_STR(XK_P), "Export current view to Postscript");
	const KeyAction* actionExportSVG = createKeyAction(XKEY_STR(XK_S), "Export current view to SVG");
	const KeyAction* actionLessColumn = createKeyAction(XKEY_STR(XK_R), "Increase Column number.");
	const KeyAction* actionMoreColumn = createKeyAction(XKEY_STR(XK_T), "Decrease Column number.");
	const KeyAction* actionShowName = createKeyAction(XKEY_STR(XK_T), "Show/Hide name.");


	char *export_dir = NULL;
	int opt;
	
	if(argc<=1) {
		usage(cerr);
		return EXIT_FAILURE;
		}

	std::string opt_str=  this->build_getopt_str();
	while ((opt = getopt(argc, argv,opt_str.c_str())) != -1) {
		switch (opt) {
		case 'o':
			export_dir = optarg;
			break;
		case 'D':
			this->cap_depth = Utils::parseInt(optarg);
			break;

		case 'f': 
			this->extend_factor = atof(optarg);
			break;
		case 's': 
			this->smooth_factor = atof(optarg);
			break;
		default: DEFAULT_HANDLE_OPTION(optopt);break;
		}
	}


	X11BamCovFileFactory samFileFactory(this);
	while(optind<argc) {
	    char* filename = argv[optind++];
	    if(Utils::endsWith(filename, ".bam")) {
		BamW* bamFile	 = (BamW*)samFileFactory.open(filename);
		if(bamFile->getSample().empty()) {
		    bamFile->sample.assign(filename);
		    }
		else
		    {
		    bamFile->sample.assign(bamFile->getSample());
		    }
		this->bams.push_back(bamFile);
		}
	    else  if(Utils::endsWith(filename, ".list"))
		{
		ifstream bamin(filename);
		if(!bamin.is_open()) {
			cerr << "Cannot open " << filename << endl;
			return EXIT_FAILURE;
			}
		string line;
		while(getline(bamin,line)) {
			if(line.empty() || line[0]=='#') continue;
			BamW* bamFile	 = (BamW*)samFileFactory.open(line.c_str());
			if(bamFile->getSample().empty()) {
				bamFile->sample.assign(line);
			    }
			else
			    {
			    bamFile->sample.assign(bamFile->getSample());
			    }
			this->bams.push_back(bamFile);
			}
		bamin.close();
		}
	    else  if(Utils::endsWith(filename, ".bed")) {
		string line;
		BedCodec bedCodec;
		ifstream regionin(filename);
		if(!regionin.is_open()) {
			cerr << "Cannot open " << filename << endl;
			return EXIT_FAILURE;
			}
		std::vector<string> tokens;
		while(getline(regionin,line)) {
			if(bedCodec.is_ignore(line)) continue;
			Utils::split('\t',line,tokens);
			string contig;
			int start;
			int end;
			if(!bedCodec.parse(tokens,&contig,&start,&end)) {
				FATAL("Cannot parse " << line);
				}
			 if(start>=end) {
			     WARN("Ignore " << line);
			     continue;
			     }
			ChromStartEnd* rgn	 = new ChromStartEnd(contig.c_str(),start,end);
			if(tokens.size()>3) rgn->label.assign(tokens[3]);
			this->regions.push_back(rgn);
			}
		regionin.close();
		}
	    else // consider interval
		{
		std::shared_ptr<Interval> rgn =  Interval::parse(filename);
		if(!rgn) FATAL("Cannot parse interval "<< filename);

		if(rgn->getStart() > rgn->getEnd()) {
		    WARN("Ignore " << filename);
		    continue;
		    }
		ChromStartEnd* interval = new ChromStartEnd(rgn->getContig(),rgn->getStart(),rgn->getEnd());
		regions.push_back(interval);
		}
	    }// end while


	if(this->bams.empty()) {
		cerr << "List of bams is empty." << endl;
		return EXIT_FAILURE;
		}
	this->num_columns = (int)std::ceil(::sqrt(this->bams.size()));
	if( this->num_columns <= 0 ) this->num_columns = 1;
	//cerr << "[DEBUG]ncols " << num_columns << endl;


	if(this->regions.empty()) {
	    cerr << "[FAILURE] List of regions is empty." << endl;
	    return EXIT_FAILURE;
	    }
	else
	    {
	    this->interval  = new ChromStartEnd(*(this->regions[0]));
	    }

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
		else if (actionPrevInterval->match(evt))
			{
			region_idx = (region_idx==0UL?regions.size()-1:region_idx-1);
			if(this->interval!=0) delete this->interval;
			this->interval = new ChromStartEnd(*(this->regions[region_idx]));
			repaint();
			}
		else if (actionNextInterval->match(evt))
			{
			region_idx = (region_idx+1>=regions.size()?0:region_idx+1);
			if(this->interval!=0) delete this->interval;
			this->interval = new ChromStartEnd(*(this->regions[region_idx]));
			repaint();
			}
		else if(actionExportPS->match(evt) || actionExportSVG->match(evt))
			{
			if(Utils::isBlank(export_dir)) {
			    WARN("export directory is not defined.");
			    }
			else
			    {
			    ostringstream os;
			    os << export_dir << (Utils::endsWith(export_dir,"/")?"":"/")
			       << this->interval->getContig()<<"_" << this->interval->getStart() << "_" << this->interval->getEnd() << "."
				<< "." << (++id_generator) << "."
				<< (actionExportPS->match(evt)?"ps":"svg");
			    std::string title =os.str();

			    ostringstream os2;
			    os2 << this->interval->getContig() <<"_" << this->interval->getStart() << "_" << this->interval->getEnd() ;
			    std::string title2 =os2.str();

			    unique_ptr<Graphics> g(actionExportPS->match(evt)?
				    (Graphics*)new PSGraphics(title.c_str(),window_width,window_height):
				    (Graphics*)new SVGraphics(title.c_str(),title2.c_str(),window_width,window_height)
				    );
			    paint(*g);
			    }
			}
		else if (actionLessColumn->match(evt) && num_columns>1)
			{
			this->num_columns--;
			repaint();
			}
		else if (actionMoreColumn->match(evt) && num_columns+1<= (int)this->bams.size())
			{
			this->num_columns++;
			repaint();
			}
		else if (actionShowName->match(evt))
			{
			this->show_sample_name = !this->show_sample_name;
			repaint();
			}
		else if(evt.type ==   Expose)
			{
			resized();
			}
		}//end while
	disposeWindow();
	return 0;
	}

int main_cnv(int argc,char** argv) {
	X11BamCov app;
	return app.doWork(argc,argv);
	}
#else

#include <iostream>
#include <cstdlib>
int main_cnv(int argc,char** argv) {
	std::cerr << argv[0] << " not available (no X11)\n";
	return EXIT_FAILURE;
	}

#endif

