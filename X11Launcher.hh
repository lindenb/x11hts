#ifndef X11LAUNCHER_HH
#define X11LAUNCHER_HH

#include <X11/Xlib.h>
#include <X11/keysym.h>
#include <X11/Xutil.h>
#include "Palette.hh"
#include "Hershey.hh"

class X11Launcher
	{
	public:
		Display *display;
		Screen *screen;
		int screen_number;
		Window window;
		int window_width;
		int window_height;
		Hershey hershey;
		Palette* palette;


		X11Launcher();
		virtual ~X11Launcher();
		virtual void resized();
		virtual void repaint()=0;
		virtual void usage(std::ostream& out);
		virtual void createWindow();
		virtual void disposeWindow();
	};

#endif
