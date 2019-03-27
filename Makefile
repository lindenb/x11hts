HTSLIB?=../htslib
LIBS= -lX11 -lm -lpthread -lhts -lz -llzma -lbz2
LDFLAGS=-L/usr/X11R6/lib -L$(HTSLIB)
INCLUDES=-I$(HTSLIB)
CFLAGS=-Wall -std=c++11 -g


test: x11hts
	find ../jvarkit-git/src/test/resources/ -name "S*.bam" > jeter.bam.list
	echo "RF01:1-1000" > jeter.bed
	echo "RF02:1-1000" >> jeter.bed
	./x11hts cnv -D 5 -B jeter.bam.list -f 0.3 -R jeter.bed

x11hts : X11Hts.cpp X11BamCov.cpp
	g++ -o $@ $(CFLAGS) $(INCLUDES) $(LDFLAGS) $^ $(LIBS)

clean:
	rm *.o x11hts
