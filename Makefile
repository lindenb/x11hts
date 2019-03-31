HTSLIB?=../htslib
LIBS= -lX11 -lm -lpthread -lhts -lz -llzma -lbz2
LDFLAGS=-L/usr/X11R6/lib -L$(HTSLIB)
INCLUDES=-I$(HTSLIB)
CFLAGS=-Wall -std=c++11 -g
CC?=g++

ifeq ($(realpath $(HTSLIB)/htslib/sam.h),)
$(error cannot find $(HTSLIB)/htslib/sam.h. Please define HTSLIB when invoking make. Something like `make HTSLIB=../htslib`)
endif
x11hts : $(addsuffix .o,X11Hts X11BamCov X11Launcher SAMRecord SAMFile)
	$(CC) -o $@ $(CFLAGS) $(INCLUDES) $(LDFLAGS) $^ $(LIBS)

X11Hts.o : X11Hts.cpp  macros.hh
	$(CC) -o $@ -c $(CFLAGS) $(INCLUDES) $< 

SAMFile.o : SAMFile.cpp SAMFile.hh
	$(CC) -o $@ -c $(CFLAGS) $(INCLUDES) $< 

X11BamCov.o : X11BamCov.cpp X11Launcher.hh SAMRecord.hh macros.hh
	$(CC) -o $@ -c $(CFLAGS) $(INCLUDES) $< 

X11Launcher.o : X11Launcher.cpp X11Launcher.hh macros.hh
	$(CC) -o $@ -c $(CFLAGS) $(INCLUDES) $< 

SAMRecord.o : SAMRecord.cpp SAMRecord.hh macros.hh
	$(CC) -o $@ -c $(CFLAGS) $(INCLUDES) $< 

macros.hh: version.hh

version.hh : 
	echo "#ifndef VERSION_HH" > $@
	echo "#define VERSION_HH" >> $@
	echo -n '#define X11HTS_VERSION "' >> $@
	-git rev-parse HEAD  | tr -d "\n"  >> $@
	echo '"' >> $@
	echo "#endif" >> $@

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
