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
		GC gc;
		void gcChanged();
	public:
		Graphics(Display* display,Drawable drawable);
		~Graphics();
		void setColor(int r,int g,int b);
		void setColor(const char* s);
		void setColor(double gray);
		void setColorForBase(char c);
		void drawLine(double x1,double y1,double x2,double y2);
		void drawRect(double x,double y,double width,double height);
		void fillRect(double x,double y,double width,double height);
		void drawChar( char c, double x, double y, double width, double height);
		void drawText( const char* s, double x, double y, double width, double height);
		void fillPolygon(size_t n,double* x,double* y);
		void drawPolygon(size_t n,double* x,double* y);

	};


#endif
