HSTLIB?=../htslib
LIBS= -lX11 -lpthread -lhts -lz -llzma -lbz2
LDFLAGS=-L/usr/X11R6/lib -L$(HSTLIB)
INCLUDES=-I$(HSTLIB)
CFLAGS=-Wall -std=c++11


x11hts : X11Hts.cpp X11BamCov.cpp
	g++ $(CFLAGS) $(INCLUDES) $(LDFLAGS) $^ $(LIBS)

clean:
	rm *.o x11hts
