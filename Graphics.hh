#ifndef GRAPHICS_HH
#define GRAPHICS_HH
#include <fstream>
#include <iostream>
#include <X11/Xlib.h>
#include <X11/keysym.h>
#include <X11/Xutil.h>

class Graphics
	{
	public:
		Graphics();
		virtual ~Graphics();
		virtual void setColor(int r,int g,int b)=0;
		virtual void setColorName(const char* s);
		virtual void setGray(double gray);
		virtual void setColorForBase(char c);
		virtual void drawLine(double x1,double y1,double x2,double y2)=0;
		virtual void drawRect(double x,double y,double width,double height)=0;
		virtual void fillRect(double x,double y,double width,double height)=0;
		virtual void drawChar( char c, double x, double y, double width, double height);
		virtual void drawText( const char* s, double x, double y, double width, double height);
		virtual void fillPolygon(size_t n,double* x,double* y)=0;
		virtual void drawPolygon(size_t n,double* x,double* y);

	};


class X11Graphics :public Graphics
	{
	private:

		Display* display;
		Drawable drawable;
		u_int64_t foreground;
		GC gc;
		void gcChanged();
	public:
		X11Graphics(Display* display,Drawable drawable);
		virtual ~X11Graphics();
		virtual void setColor(int r,int g,int b);
		virtual void drawLine(double x1,double y1,double x2,double y2);
		virtual void drawRect(double x,double y,double width,double height);
		virtual void fillRect(double x,double y,double width,double height);
		virtual void fillPolygon(size_t n,double* x,double* y);
	};

class PSGraphics :public Graphics
	{
	private:
	    std::ofstream out;
	    int height;
	    double flipY(double y);
	    double toInch(double y);


	public:
		PSGraphics(const char* fname,int width,int height);
		virtual ~PSGraphics();
		virtual void setColor(int r,int g,int b);
		virtual void drawLine(double x1,double y1,double x2,double y2);
		virtual void drawRect(double x,double y,double width,double height);
		virtual void fillRect(double x,double y,double width,double height);
		virtual void fillPolygon(size_t n,double* x,double* y);
	};

class SVGraphics :public Graphics
	{
	private:
	    std::ofstream out;
	    std::string color;
		virtual void _polygon(size_t n,double* x,double* y,const char* style);
		virtual void _rect(double x,double y,double width,double height,const char* style);
	public:
		SVGraphics(const char* fname,const char* title,int width,int height);
		virtual ~SVGraphics();
		virtual void setColor(int r,int g,int b);
		virtual void drawLine(double x1,double y1,double x2,double y2);
		virtual void drawRect(double x,double y,double width,double height);
		virtual void fillRect(double x,double y,double width,double height);
		virtual void drawText( const char* s, double x, double y, double width, double height);
		virtual void drawPolygon(size_t n,double* x,double* y);
		virtual void fillPolygon(size_t n,double* x,double* y);
	};

#endif
