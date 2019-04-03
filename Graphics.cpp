#include <iostream>
#include <fstream>
#include <string>
#include <map>
#include "Utils.hh"
#include "Hershey.hh"

#include "Graphics.hh"
#include "macros.hh"

using namespace std;

//https://github.com/ConnerTenn/LightShades/blob/813ffee8ea83f6db0499bee4ca030f301f8f1fba/Code/WindowController.cpp

struct RGBColor
	{
	int r,g,b;
	};

#define X11_RGB_TXT "/usr/share/X11/rgb.txt"
static map<string,RGBColor> init_name2color() {
    map<string,RGBColor> m;
    ifstream in(X11_RGB_TXT);
    if(in.is_open())
	{
	string line;
	while(getline(in,line,'\n')) {
	    line=Utils::normalize_space(line);
	    if(Utils::isBlank(line)) continue;
	    if(Utils::startsWith(line, "#")) continue;
	    if(Utils::startsWith(line, "!")) continue;
	    vector<string> tokens;
	    Utils::split(' ', line, tokens,4);
	    if(tokens.size()!=4) {
		cerr << "Bad color in " << line << endl;
		continue;
		}
	    RGBColor c;
	    c.r = atoi(tokens[0].c_str());
	    c.g = atoi(tokens[1].c_str());
	    c.b = atoi(tokens[2].c_str());
	    m.insert(make_pair(tokens[3], c));
	    }
	in.close();
	}
    else
	{
	WARN("Cannot open "<< X11_RGB_TXT);
	}
    return m;
    }

static map<string,RGBColor> name2color = init_name2color();

static RGBColor* find_color_by_name(const char *s) {
    if(s==NULL) return NULL;
    map<string,RGBColor>::iterator r = name2color.find(s);
    if(r==name2color.end()) {
	WARN("Cannot find color "<< s);
	return NULL;
    }
    return &(r->second);
    }

Graphics::Graphics(Display* display,Drawable drawable):
	display(display),
	drawable(drawable),
	foreground(0)
    {
    gc=::XCreateGC(this->display, this->drawable,0,0);
    }

void Graphics::setColor(int r,int g,int b) {
    this->foreground = (b<<8*0) | (g<<8*1) | (r<<8*2) | (0xff<<8*3);
    gcChanged();
    }

void Graphics::setColor(const char* s) {
    RGBColor* c= find_color_by_name(s);
    if(c==NULL) { return;  }
    setColor(c->r,c->g,c->b);
    }
void Graphics::setColor(double gray) {
    if(gray<0) gray=0;
    if(gray>1) gray=1;
    int g=(int)(gray*255);
    setColor(g,g,g);
    }


void Graphics::gcChanged() {
    XGCValues values;
    ::XFreeGC(this->display,this->gc);
    values.foreground = this->foreground;
    unsigned long mask = GCForeground;
    this->gc = ::XCreateGC(this->display, this->drawable, mask, &values);
    }


Graphics::~Graphics(){
    ::XFreeGC(this->display,this->gc);
    ::XFlush(this->display);

}
void Graphics::drawLine(double x1,double y1,double x2,double y2) {
    XDrawLine(display,drawable,gc,(int)x1,(int)y1,(int)x2,(int)y2);
    }

void Graphics::drawRect(double x,double y,double width,double height){
    XDrawRectangle(display,drawable,gc,(int)x,(int)y,(int)width,(int)height);
}
void Graphics::fillRect(double x,double y,double width,double height){
    XFillRectangle(display,drawable,gc,(int)x,(int)y,(int)width,(int)height);
    }
void Graphics::drawChar(
			char c,
			double x, double y,
			double width, double height
			)
    {
    if(isspace(c)) return;
    char tmp[2]={c,0};
    drawText(tmp,x,y,width,height);
    }

void Graphics::drawText(
			const char* s,
			double x, double y,
			double width, double height
			)
    {
    Hershey hershey;
    hershey.paint(this,s,x,y,width,height);
    }
void Graphics::fillPolygon(size_t n,double* x,double* y)
    {
    if(n<2) return;
    XPoint* pt=new XPoint[n];
    for(size_t i=0;i<n;i++)
	{
	pt[i].x=(int)x[i];
	pt[i].y=(int)y[i];
	}
    ::XFillPolygon(this->display,this->drawable, gc, pt, (int)n, Complex,CoordModeOrigin);
    delete[] pt;
    }

void Graphics::drawPolygon(size_t n,double* x,double* y)
    {
    if(n<2) return;
    for(size_t i=0;i+1<n;i++)
	{
	drawLine(x[i], y[i], x[i+1], y[i+1]);
	}
    drawLine(x[n-1], y[n-1], x[0], y[0]);
    }

void Graphics::setColorForBase(char c) {
	switch(toupper(c))
	    {
	    case 'N': setColor("black");break;
	    case 'A': setColor("red");break;
	    case 'T': setColor("green");break;
	    case 'G': setColor("yellow");break;
	    case 'C': setColor("blue");break;
	    default:  setColor("orange");break;
	    }
    }
