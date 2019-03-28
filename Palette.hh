#ifndef PALETTE_H
#define PALETTE_H
#include <X11/Xlib.h>
#include <vector>
#include <cstdio>

#define NAMEDCOLOR(A) XAllocNamedColor(display, scr_cmap,#A, &A, &A);
class Palette
	{
public:
	XColor red,green,blue,yellow,dark_slate_gray;
    std::vector<XColor> grays;

	// cat /usr/share/X11/rgb.txt
	Palette( Display* display, int screen_number) {
		Colormap scr_cmap = DefaultColormap(display,screen_number);
		NAMEDCOLOR(blue);
		NAMEDCOLOR(red);
		NAMEDCOLOR(green);
		NAMEDCOLOR(yellow);
		 XAllocNamedColor(display, scr_cmap, "dark slate gray", &dark_slate_gray, &dark_slate_gray);

		 for(int i=0;i<=100;i++)
		 	 {
			 XColor g;
			 char tmp[10];
			 sprintf(tmp,"gray%d",i);
			 XAllocNamedColor(display, scr_cmap,tmp,&g,&g);
			 grays.push_back(g);
		 	 }
		}
	XColor& gray(int i) {
		if(i<0) i=0;
		if(i>(int)this->grays.size()) i=(int)this->grays.size()-1;
		return this->grays[i];
		}
	XColor& gray(double f) {
		return this->gray((int)(f*this->grays.size()));
		}
	};

#undef NAMEDCOLOR

#endif
