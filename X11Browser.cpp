#include <map>
#include <algorithm>
#include <vector>
#include <cstdlib>
#include "version.hh"
#include "SAMRecord.hh"
#include "X11Launcher.hh"
#include "macros.hh"

using namespace std;

class X11Browser;

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
			int i = first->getAlignmentStart();
			if(second!=0 ) {
				i=std::min(i,second->getAlignmentStart());
			}
			return i;
		}
		int getEnd() const {
			int i = first->getAlignmentEnd();
			if(second!=0 ) {
				i=std::max(i,second->getAlignmentEnd());
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
	public:
		std::vector<SAMFile*> bams;
		std::vector<Interval*> intervals;
		std::vector<vector<PairOrReads*>*> rows;
		Interval* interval;
		size_t bam_index;
		size_t region_idx;
		bool group_by_pair;
		X11Browser();
		virtual ~X11Browser();
		virtual double pos2pixel(int pos);
		virtual int getStart( SAMRecord* rec);
		virtual int getEnd( SAMRecord* rec);
		virtual int doWork(int argc,char** argv);
		virtual void repaint();
		virtual void paint();
	};

X11Browser::X11Browser():interval(0),bam_index(0),region_idx(0),group_by_pair(true) {
	interval = new Interval;
	interval->contig.assign("undef");
	interval->start=1;
	interval->end=1;
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

double X11Browser::pos2pixel(int pos) {
	return (pos-interval->start)/((double)interval->length())* window_width;
}

void X11Browser::paint() {
	GC gc = ::XCreateGC(this->display, this->window, 0, 0);
	XSetForeground(this->display, gc, WhitePixel(this->display, this->screen_number));
	::XFillRectangle(this->display,this->window, gc,0,0,this->window_width,this->window_height);
	double y=0;
	double dy=10;
	for(size_t y_row=0;y_row< rows.size() && y< this->window_height ;++y_row) {
		vector<PairOrReads*>* row=rows[y_row];
		for(size_t por_idx=0;por_idx<row->size();++por_idx) {
			PairOrReads* por = row->at(por_idx);
		}
		y+=dy;
	}
	XFlush(display);
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

int X11Browser::doWork(int argc,char** argv)
	{
	char* bam_list = NULL;
	char* region_list = NULL;
	int opt;

	if(argc<=1) {
		usage(cerr);
		return EXIT_FAILURE;
		}

	while ((opt = getopt(argc, argv, "B:R:vh")) != -1) {
		switch (opt) {
		case 'h':
			usage(cout);
			return 0;
		case 'v':
			cout << X11HTS_VERSION << endl;
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
	//
	if(bam_list == NULL) FATAL("List of bams is undefined.");

	ifstream bamin(bam_list);
	if(!bamin.is_open()) {
		cerr << "Cannot open " << bam_list << endl;
		return EXIT_FAILURE;
		}
	string line;
	SAMFileFactory samFileFactory;
	while(getline(bamin,line)) {
		if(line.empty() || line[0]=='#') continue;
		SAMFile* bamFile	 = samFileFactory.open(line.c_str());
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
	BedCodec bedCodec;
	bedCodec.parseBedFile(region_list,this->intervals);
	if(this->intervals.empty())  FATAL("List of regions is empty.");


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
				region_idx = (region_idx==0UL?intervals.size()-1:region_idx-1);
				repaint();
				}
			else if (evt.xkey.keycode == XKeysymToKeycode(this->display, XK_Right))
				{
				region_idx = (region_idx+1>=intervals.size()?0:region_idx+1);
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

int main_ browser(int argc,char** argv) {
	X11Browser app;
	return app.doWork(argc,argv);
	}
