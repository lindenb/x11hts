HTSLIB?=../htslib
LIBS= -lX11 -lm -lpthread -lhts -lz -llzma -lbz2
LDFLAGS=-L/usr/X11R6/lib -L$(HTSLIB)
INCLUDES=-I$(HTSLIB)
CFLAGS=-Wall -std=c++11 -g


ifeq ($(realpath $(HTSLIB)/htslib/sam.h),)
$(error cannot find $(HTSLIB)/htslib/sam.h. Please define HTSLIB when invoking make. Something like `make HTSLIB=../htslib`)
endif
x11hts : X11Hts.cpp X11BamCov.cpp
	g++ -o $@ $(CFLAGS) $(INCLUDES) $(LDFLAGS) $^ $(LIBS)

test: x11hts
	find ${HOME}/src/jvarkit-git/src/test/resources/ -name "S*.bam" > jeter.bam.list
	find ${HOME}/src/jvarkit-git/src/test/resources/ -name "S*.bam" >> jeter.bam.list
	find ${HOME}/src/jvarkit-git/src/test/resources/ -name "S*.bam" >> jeter.bam.list
	echo "RF01:1-2000 pim" > jeter.bed
	echo "RF02:1-1000	PAM" >> jeter.bed
	echo "RF03	1	1000	POUM" >> jeter.bed
	./x11hts cnv -D 5 -B jeter.bam.list -f 0.3 -R jeter.bed

clean:
	rm *.o x11hts
