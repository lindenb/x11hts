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

Graphics::Graphics() {
}

Graphics::~Graphics() {
}

void Graphics::setColorForBase(char c) {
	switch(toupper(c))
	    {
	    case 'N': setColorName("black");break;
	    case 'A': setColorName("red");break;
	    case 'T': setColorName("green");break;
	    case 'G': setColorName("yellow");break;
	    case 'C': setColorName("blue");break;
	    default:  setColorName("orange");break;
	    }
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

void Graphics::setColorName(const char* s) {
    RGBColor* c= find_color_by_name(s);
    if(c==NULL) { return;  }
    setColor(c->r,c->g,c->b);
    }

void Graphics::setGray(double gray) {
    if(gray<0) gray=0;
    if(gray>1) gray=1;
    int g=(int)(gray*255);
    setColor(g,g,g);
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

void Graphics::drawPolygon(size_t n,double* x,double* y)
    {
    if(n<2) return;
    for(size_t i=0;i+1<n;i++)
	{
	drawLine(x[i], y[i], x[i+1], y[i+1]);
	}
    drawLine(x[n-1], y[n-1], x[0], y[0]);
    }



X11Graphics::X11Graphics(Display* display,Drawable drawable):
	display(display),
	drawable(drawable),
	foreground(0)
    {
    gc=::XCreateGC(this->display, this->drawable,0,0);
    }

void X11Graphics::setColor(int r,int g,int b) {
    this->foreground = (b<<8*0) | (g<<8*1) | (r<<8*2) | (0xff<<8*3);
    gcChanged();
    }



void X11Graphics::gcChanged() {
    XGCValues values;
    ::XFreeGC(this->display,this->gc);
    values.foreground = this->foreground;
    unsigned long mask = GCForeground;
    this->gc = ::XCreateGC(this->display, this->drawable, mask, &values);
    }


X11Graphics::~X11Graphics(){
    ::XFreeGC(this->display,this->gc);
    ::XFlush(this->display);

}
void X11Graphics::drawLine(double x1,double y1,double x2,double y2) {
    XDrawLine(display,drawable,gc,(int)x1,(int)y1,(int)x2,(int)y2);
    }

void X11Graphics::drawRect(double x,double y,double width,double height){
    XDrawRectangle(display,drawable,gc,(int)x,(int)y,(int)width,(int)height);
}
void X11Graphics::fillRect(double x,double y,double width,double height){
    XFillRectangle(display,drawable,gc,(int)x,(int)y,(int)width,(int)height);
    }




void X11Graphics::fillPolygon(size_t n,double* x,double* y)
    {
    if(n<2) return;
    XPoint* pt=new XPoint[n];
    size_t i=0;
    while(i<n)
	{
	pt[i].x=(int)x[i];
	pt[i].y=(int)y[i];
	++i;
	}
    ::XFillPolygon(this->display,this->drawable, gc, pt, (int)n, Complex,CoordModeOrigin);
    delete[] pt;
    }

/* ================================================================================== */

PSGraphics::PSGraphics(const char* fname,int width,int height):out(fname),height(height) {
    if(!out.is_open()) {
	FATAL("Cannot open "<< fname << "." << strerror(errno));
	}
     out << "%!PS-Adobe-3.0 EPSF-3.0" << endl;
     out << "%%Creator: "<< __FILE__ << endl;
     out << "%%BoundingBox: 0 0 " << toInch(width)<<" "<<toInch(height) << endl;
     out << "%%Pages: 1" << endl;
     out << "%%Page: 1 1" << endl;
    }
PSGraphics::~PSGraphics()
    {
    out << "\nshowpage" << endl;
    out << "%EOF" << endl;
    out.flush();
    out.close();
    }
double PSGraphics::flipY(double y) {
    return this->height - y;
}
double PSGraphics::toInch(double v) {
    return  v/72.0;
}

void PSGraphics::setColor(int r,int g,int b) {
    out << (r/255.0) << " " << (g/255.0) << " "<< (b/255.0)<<" setrgbcolor ";
    }

void PSGraphics::drawLine(double x1,double y1,double x2,double y2) {
    out << " ";
    out << toInch(x1);
    out << " ";
    out << toInch(flipY(y1));
    out << " moveto ";
    out << toInch(x2);
    out << " ";
    out << toInch(flipY(y2));
    out << " lineto stroke ";
}
void PSGraphics::drawRect(double x,double y,double width,double height) {
	if(width<=0 || height<=0) return;
	double xv[4]= {x,x+width,x+width,x};
	double yv[4]= {y,y,y+height,y+height};
	drawPolygon(4,xv, yv);
    }

void PSGraphics::fillRect(double x,double y,double width,double height) {
    if(width<=0 || height<=0) return;
    double xv[4]= {x,x+width,x+width,x};
    double yv[4]= {y,y,y+height,y+height};
    fillPolygon(4,xv, yv);
}
void PSGraphics::fillPolygon(size_t n,double* x,double* y) {
    for(size_t j=0;j< n;++j) {
	out << toInch(x[j]) << " " << toInch(flipY(y[j]));
	out << (j==0?" moveto ":" lineto ");
	}
    out <<" closepath fill ";
    }

/* ================================================================================== */


SVGraphics::SVGraphics(const char* fname,const char* title,int width,int height):out(fname),color("black") {
    if(!out.is_open()) {
	FATAL("Cannot open "<< fname << "." << strerror(errno));
	}
     out << "<xml version=\"1.0\" encoding=\"UTF-8\"?>" << endl;
     out << "<svg xmlns=\"http://www.w3.org/2000/svg\" version=\"1.1\" width=\"" << (width+1) << "\" height=\"" << (height+1) << "\">" << endl;
     if(Utils::isBlank(title)) out << "<title>" << Utils::escapeXml(title) << "</title>";
     out << "<g>";
    }
SVGraphics::~SVGraphics()
    {
    out << "</g>\n</svg>" << endl;
    out.flush();
    out.close();
    }


void SVGraphics::setColor(int r,int g,int b) {
    std::ostringstream os;
    os << "rgb("<<r<<","<<g<<","<<b<<")";
    this->color.assign(os.str());
    }
#define SVG_ATT(T,V) T << "=\"" << V << "\""

void SVGraphics::drawLine(double x1,double y1,double x2,double y2) {
    out << "<line " << SVG_ATT("x1",x1)
	<<  " "
	<< SVG_ATT("y1",y1)
	<<  " "
	<< SVG_ATT("x2",x2)
	<<  " "
	<< SVG_ATT("y2",y2)
	<< " style=\"stroke:" << this->color
	<< "\"/>"
	;
    }

void SVGraphics::_rect(double x,double y,double width,double height,const char* style) {
    if(width<=0 || height<=0) return;
    out << "<rect "
	<< SVG_ATT("x",x)
	<<  " "
	<< SVG_ATT("y",y)
	<<  " "
	<< SVG_ATT("width",width)
	<<  " "
	<< SVG_ATT("height",height)
	<< " style=\""<< style << ":" << this->color
	<< "\"/>"
	;
    }


void SVGraphics::drawRect(double x,double y,double width,double height) {
    _rect(x,y,width,height,"stroke");
    }

void SVGraphics::fillRect(double x,double y,double width,double height) {
    _rect(x,y,width,height,"fill");
    }

void SVGraphics::_polygon(size_t n,double* x,double* y,const char* style) {
    out << "<path d=\"";
    for(size_t j=0;j< n;++j) {
	out << (j==0?"M ":"L");
	out << x[j] << " " <<  (y[j]);
	}
    out << " Z\" style=\"" << style << ":" << this->color
    	<< "\"/>"
	;
    }
void SVGraphics::drawPolygon(size_t n,double* x,double* y) {
    _polygon(n,x,y,"stroke");
    }

void SVGraphics::fillPolygon(size_t n,double* x,double* y) {
    _polygon(n,x,y,"fill");
    }

void SVGraphics::drawText( const char* s, double x, double y, double width, double height) {
    Hershey hershey;
    out << "<path d=\"";
    hershey.svgPath(out,s, x, y, width, height);
    out << "\" style=\"stroke:" << this->color << "\"/>";
    }

#undef SVG_ATT

/* ================================================================================== */
NullGraphics::NullGraphics() {
}

NullGraphics::~NullGraphics() {
}

void NullGraphics::setColor(int r,int g,int b) {
}
void NullGraphics::drawLine(double x1,double y1,double x2,double y2) {
}
void NullGraphics::drawRect(double x,double y,double width,double height) {
}
void NullGraphics::fillRect(double x,double y,double width,double height) {
}
void NullGraphics::fillPolygon(size_t n,double* x,double* y) {
}
