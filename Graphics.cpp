#include "Graphics.hh"

//https://github.com/ConnerTenn/LightShades/blob/813ffee8ea83f6db0499bee4ca030f301f8f1fba/Code/WindowController.cpp
#define BEGIN_GC \
		XGCValues values;\
		values.foreground = this->foreground;\
		unsigned long mask = GCForeground;\
		GC gc = ::XCreateGC(this->display, this->drawable, mask, &values)

#define END_GC ::XFreeGC(display,gc)

Graphics::Graphics(Display* display,Drawable drawable):display(display),drawable(drawable),foreground(0){

}
Graphics::~Graphics(){
	XFlush(display);
}
void Graphics::drawLine(double x1,double y1,double x2,double y2) {
	BEGIN_GC;
	XDrawLine(display,drawable,gc,(int)x1,(int)y1,(int)x2,(int)y2);
	END_GC;
}
void Graphics::drawRect(double x,double y,double width,double height){
	BEGIN_GC;
	XDrawRectangle(display,drawable,gc,(int)x,(int)y,(int)width,(int)height);
	END_GC;
}
void Graphics::fillRect(double x,double y,double width,double height){
	BEGIN_GC;
	XFillRectangle(display,drawable,gc,(int)x,(int)y,(int)width,(int)height);
	END_GC;
}
