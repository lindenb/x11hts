.PHONY=all
HTSLIB?=../htslib
LIBS= -lX11 -lm -lpthread -lhts -lz -llzma -lbz2 -lm
LDFLAGS=-L/usr/X11R6/lib -L$(HTSLIB)
INCLUDES=-I$(HTSLIB)
CFLAGS=-Wall -std=c++11 -g
CC=g++


OBJS=X11Hts X11BamCov X11Launcher SAMRecord SAMFile AbstractCmdLine Utils SplitFastq GZipInputStreamBuf \
	InterleavedFastq X11Browser Graphics BedLine Faidx Locatable Interval Hershey KString FindNNNInFasta

ifeq ($(realpath $(HTSLIB)/htslib/sam.h),)
$(error cannot find $(HTSLIB)/htslib/sam.h. Please define HTSLIB when invoking make. Something like `make HTSLIB=../htslib`)
endif

%.o: %.cpp
	$(CC) -o $@ -c $(CFLAGS) $(INCLUDES) $< 

all: x11hts

x11hts : version.hh $(addsuffix .o,$(OBJS))
	$(CC) -o $@ $(CFLAGS) $(INCLUDES) $(LDFLAGS) $(addsuffix .o,$(OBJS)) $(LIBS)

X11Hts.o : X11Hts.cpp 
	$(CC) -o $@ -c $(CFLAGS) $(INCLUDES) $< 

SAMFile.o : SAMFile.cpp SAMFile.hh
	$(CC) -o $@ -c $(CFLAGS) $(INCLUDES) $< 

X11BamCov.o : X11BamCov.cpp X11Launcher.hh SAMRecord.hh macros.hh
	$(CC) -o $@ -c $(CFLAGS) $(INCLUDES) $< 

X11Launcher.o : X11Launcher.cpp X11Launcher.hh macros.hh
	$(CC) -o $@ -c $(CFLAGS) $(INCLUDES) $< 

SAMRecord.o : SAMRecord.cpp SAMRecord.hh macros.hh
	$(CC) -o $@ -c $(CFLAGS) $(INCLUDES) $< 

InterleavedFastq.o: InterleavedFastq.cpp AbstractCmdLine.o
	$(CC) -o $@ -c $(CFLAGS) $(INCLUDES) $<

SplitFastq.o : SplitFastq.cpp
	$(CC) -o $@ -c $(CFLAGS) $(INCLUDES) $< 

GZipInputStreamBuf.o : GZipInputStreamBuf.cpp GZipInputStreamBuf.hh
	$(CC) -o $@ -c $(CFLAGS) $(INCLUDES) $< 

AbstractCmdLine.o: AbstractCmdLine.cpp AbstractCmdLine.hh
	$(CC) -o $@ -c $(CFLAGS) $(INCLUDES) $< 

Utils.o: Utils.cpp Utils.hh
	$(CC) -o $@ -c $(CFLAGS) $(INCLUDES) $< 

X11Browser.o : X11Browser.cpp
	$(CC) -o $@ -c $(CFLAGS) $(INCLUDES) $< 

Graphics.o: Graphics.cpp Graphics.hh
	$(CC) -o $@ -c $(CFLAGS) $(INCLUDES) $< 

BedLine.o: BedLine.cpp BedLine.hh
	$(CC) -o $@ -c $(CFLAGS) $(INCLUDES) $< 
Faidx.o: Faidx.cpp Faidx.hh
	$(CC) -o $@ -c $(CFLAGS) $(INCLUDES) $< 
Interval.o: Interval.cpp Interval.hh
	$(CC) -o $@ -c $(CFLAGS) $(INCLUDES) $< 
Locatable.o: Locatable.cpp Locatable.hh
	$(CC) -o $@ -c $(CFLAGS) $(INCLUDES) $<




macros.hh: version.hh

version.hh : 
	echo "#ifndef VERSION_HH" > $@
	echo "#define VERSION_HH" >> $@
	echo -n '#define X11HTS_VERSION "' >> $@
	-git rev-parse HEAD  | tr -d "\n"  >> $@
	echo '"' >> $@
	echo "#endif" >> $@

test3: x11hts
	echo "3	38674526	38687267" > jeter.bed
	echo "/home/lindenb/CD07595.bam" > jeter.bam.list
	./x11hts browse  jeter.bam.list   jeter.bed  /home/lindenb/data/human_g1k_v37.fasta


test2: x11hts
	find ${HOME}/src/jvarkit-git/src/test/resources/ -name "S*.bam" |sort> jeter.bam.list
	echo "RF03	1	10	POUM" > jeter.bed
	echo "RF03	1	1000	POUM" >> jeter.bed
	./x11hts browse -B jeter.bam.list  -r jeter.bed -R /home/lindenb/src/jvarkit-git/src/test/resources/rotavirus_rf.fa

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
