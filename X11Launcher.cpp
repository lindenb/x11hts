#include "X11Launcher.hh"
#include "Utils.hh"
#include "macros.hh"
using namespace std;

X11Launcher::X11Launcher():display(0),screen(0),
		screen_number(0),
		window(0),
		window_width(0),
		window_height(0),
		offscreen(0)
{
}

X11Launcher::~X11Launcher() {
    for(auto a: this->all_key_actions) delete a;

}

void X11Launcher::key_usage(std::ostream& out) {
    AbstractCmd::usage(out);

    out << "## Keys" << endl;
    for(auto a: this->all_key_actions) {
	out << "< "<<a->keydef << " > " << a->description << endl;
    }

    out << endl;
}


void X11Launcher::usage(std::ostream& out) {
    AbstractCmd::usage(out);
    key_usage(out);
    }


void X11Launcher::resized() {
	//int x,y,wr;
	//unsigned int w,h,bw, d;
	 XWindowAttributes att;
	::XGetWindowAttributes(display, window, &att);
	if(att.width!=this->window_width || att.height!=this->window_height) {
		this->window_width = att.width;
		this->window_height =  att.height;
		if(this->offscreen != 0) XFreePixmap(display,offscreen);
		this->offscreen = 0;
		this->repaint();
		}
	}

Pixmap X11Launcher::getOffscreen() {
	if(this->offscreen==0) {
		int depth = DefaultDepthOfScreen(this->screen);
		this-> offscreen = XCreatePixmap(display, window,this->window_width,this->window_height, depth);
		}
	return offscreen;
	}

void X11Launcher::createWindow() {
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
	if(offscreen!=0) XFreePixmap(display,offscreen);
	::XUnmapWindow(display,window);
	::XCloseDisplay(display);
	display=NULL;
	window=0;
	window_height=0;
	window_width=0;
	}



X11Launcher::KeyAction* X11Launcher::createKeyAction( KeySym keysim,const char* keydef,const char* description)
    {
    for(auto a: all_key_actions) {
	if(a->keysim==keysim) {
	    FATAL("Duplicate keysym registered " << keydef << " "<< description << "/"<< a->description);
	}
    }
    X11Launcher::KeyAction* k = new X11Launcher::KeyAction(this,keysim,keydef,description);
    this->all_key_actions.push_back(k);
    return k;
    }

X11Launcher::KeyAction::KeyAction( X11Launcher* owner,KeySym keysim,const char* keydef,const char* description):
    owner(owner),
    keysim(keysim),
    keydef(keydef),
    description(description)
    {
    if(Utils::startsWith(this->keydef,"XK_")) this->keydef.erase(0,3);
    }

bool X11Launcher::KeyAction::match(const XEvent& evt)  const {
    return evt.type ==  KeyPress &&
	   evt.xkey.keycode == ::XKeysymToKeycode(
	       this->owner->display,
	       this->keysim
	       );
    }
