#ifndef GRAPHICS_HH
#define GRAPHICS_HH
#include <X11/Xlib.h>
#include <X11/keysym.h>
#include <X11/Xutil.h>



class Graphics
	{
	private:
		Display* display;
		Drawable drawable;
		u_int64_t foreground;
	public:
		Graphics(Display* display,Drawable drawable);
		~Graphics();
		void drawLine(double x1,double y1,double x2,double y2);
		void drawRect(double x,double y,double width,double height);
		void fillRect(double x,double y,double width,double height);
	};


#endif
