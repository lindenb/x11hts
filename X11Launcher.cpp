#include "X11Launcher.hh"
#include "macros.hh"
using namespace std;

X11Launcher::X11Launcher():display(0),screen(0),
		screen_number(0),
		window(0),
		window_width(0),
		window_height(0)
{
}

X11Launcher::~X11Launcher() {

}

void X11Launcher::resized() {
	//int x,y,wr;
	//unsigned int w,h,bw, d;
	 XWindowAttributes att;
	::XGetWindowAttributes(display, window, &att);
	if(att.width!=this->window_width || att.height!=this->window_height) {
		this->window_width = att.width;
		this->window_height =  att.height;
		this->repaint();
		}
	}

void X11Launcher::createWindow() {
	//
	this->display = ::XOpenDisplay(NULL);
	if (this->display == NULL) FATAL("Cannot open display.");

	 this->screen_number = DefaultScreen(this->display);


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
}

void X11Launcher::disposeWindow() {
	::XUnmapWindow(display,window);
	::XCloseDisplay(display);
	display=NULL;
	window=0;
	window_height=0;
	window_width=0;
	}
