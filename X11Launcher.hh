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
#ifndef X11LAUNCHER_HH
#define X11LAUNCHER_HH

#include <map>
#include <functional>
#include <X11/Xlib.h>
#include <X11/keysym.h>
#include <X11/Xutil.h>
#include "AbstractCmdLine.hh"



#define XKEY_STR(a) a,#a

class X11Launcher:public AbstractCmd
	{
	public:
		class KeyAction
		    {
		    public:
			    X11Launcher* owner;
			    KeySym keysim;
			    std::string keydef;
			    std::string description;
			    KeyAction( X11Launcher* owner,KeySym keysim,const char* keydef,const char* description);
			    bool match(const XEvent& evt) const;
		    };


		Display *display;
		Screen *screen;
		int screen_number;
		Window window;
		int window_width;
		int window_height;

		X11Launcher();
		virtual ~X11Launcher();
		virtual void usage(std::ostream& out);
		virtual void key_usage(std::ostream& out);
		virtual void resized();
		virtual void repaint()=0;
		virtual void createWindow();
		virtual void disposeWindow();
	protected:
		KeyAction* createKeyAction( KeySym keysim,const char* keydef,const char* description);
		std::vector<KeyAction*> all_key_actions;
	};

#endif
