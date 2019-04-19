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
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <limits.h>
#include <htslib/sam.h>

#define FACTOR 10.0

/**

This tools calculate the mean depth of a bam (ignoring the non-covered bases)
and output a BED file of the regions covered more than mean-depth*FACTOR

gcc -o a.out -O3 -Wall -I../htslib src.c -L../htslib  -lm -lpthread -lhts -lz -llzma -lbz2

 */

struct StepInfo
    {
    int min_mapQ;
    htsFile* fp;
    bam_hdr_t* hdr;
    };

// This function reads a BAM alignment from one BAM file.
static int read_bam(void *data, bam1_t *b) // read level filters better go here to avoid pileup
{
    struct StepInfo *info = (struct StepInfo*)data;
    int ret = -1;
    for(;;)
    {
        ret = sam_read1(info->fp, info->hdr, b);
        if ( ret<0 ) break;
        if ( b->core.flag & (BAM_FUNMAP | BAM_FSECONDARY | BAM_FQCFAIL | BAM_FDUP) ) continue;
        if ( (int)b->core.qual < info->min_mapQ ) continue;
        break;
    }
    return ret;
}
#define WHERE fprintf(stderr,"%d\n",__LINE__)

#define BUFFER_SIZE 2000000
int main(int argc,char**argv) {
    int step;
    char* fn=NULL;
    bam_mplp_t mplp;
    struct StepInfo info;
    info.min_mapQ = 10;
    double mean_cov = 0.0;
    char* buffer ;

    if(argc!=2) {
	fprintf(stderr,"Usage: %s file.bam > cov.bed\n",argv[0]);
	return EXIT_FAILURE;
    }
    fn = argv[1];

    buffer= malloc(sizeof(char)*BUFFER_SIZE);
    if(buffer==NULL) {
	fprintf(stderr,"out of memory\n");
	return EXIT_FAILURE;
    }
    setvbuf ( stdout , buffer , _IOFBF ,BUFFER_SIZE);

    for(step=0;step<2; ++step) //step 0 : get min/max/mean
	{
	struct StepInfo* info_ptr=&info;
	const bam_pileup1_t **plp = (const bam_pileup1_t**)calloc(1, sizeof(bam_pileup1_t*));
	int tid = 0;
	int pos = 0;
	int depth = 0;

	info.fp = hts_open(fn, "rb");
	if(info.fp==NULL) {
	    fprintf(stderr,"Cannot open %s %s",fn,strerror(errno));
	    return EXIT_FAILURE;
	    }
	info.hdr = sam_hdr_read(info.fp);
	if(info.hdr==NULL) {
	    fprintf(stderr,"Cannot read header for %s %s",fn,strerror(errno));
	    return EXIT_FAILURE;
	    }
	mplp = bam_mplp_init(1, read_bam, (void**)&info_ptr); // initialization

	if(step==0)
	    {
	    double sum_cov = 0.0;
	    long count_reads = 0;

	    while(bam_mplp_auto(mplp,&tid,&pos,&depth,plp)>0)
		{
		sum_cov+=depth;
		++count_reads;
		}
	    mean_cov = sum_cov/count_reads;
	    fprintf(stderr,"%s\t%ld\t%f\n",fn,count_reads,mean_cov);

	    }
	else
	    {
	    int max_depth = (int)(mean_cov * FACTOR );
	    int bed_tid = -1;
	    int bed_start = -1;
	    int bed_end = -1;
	    for(;;)
		{
		int ret = bam_mplp_auto(mplp,&tid,&pos,&depth,plp);

		if(ret<=0 || /* EOF */
		    depth < max_depth || /* Not high-cov region, close the bed */
		   (bed_tid>=0 && bed_tid != tid) || /* new contig */
		   (bed_tid>=0 && bed_end!=pos) /*closed bed */
		    ) {
		    if(bed_tid>=0 && bed_start<bed_end) {
			fputs(info.hdr->target_name[bed_tid],stdout);\
			printf("\t%d", bed_start);\
			printf("\t%d\n", bed_end);

			bed_tid = -1;
			bed_start = -1;
			bed_end = -1;
			}
		    if(ret<=0) break;
		    }




		if(depth >= max_depth)
		    {
		    if(bed_tid==-1) {
			bed_tid = tid;
			bed_start = pos;
			}
		    bed_end = pos+1;
		    }
		}
	    }

	bam_mplp_destroy(mplp);
	bam_hdr_destroy(info.hdr);
	hts_close(info.fp);
	free(plp);
	}
    fflush(stdout);
    fclose(stdout);
    free(buffer);
  return EXIT_SUCCESS;
  }
