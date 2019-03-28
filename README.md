![works on my machine](https://img.shields.io/badge/works-on%20my%20machine-green.png)

![Last commit](https://img.shields.io/github/last-commit/lindenb/x11hts.png)

# x11hts

X11 related utilities for hts

## Author

Pierre Lindenbaum PhD / @yokofakun 2019

## Compilation

dependencies:

  * **X11/dev** libraries 
  * **htslib** :  https://github.com/samtools/htslib 

compile using make, specifying the path to HTSLIB

```
 $ make HTSLIB=${HOME}/packages/htslib
 ```

should generate a program named `x11hts`

sometimes you may have to specify the env variable `LD_LIBRARY_PATH`

```
export LD_LIBRARY_PATH=/home/lindenb/packages/htslib
```



# CNV
  Displays Bam coverage

## Example

```
./x11hts cnv -B bam.list -f 0.3 -R input.bed
```


## Keys

  * 'S' save current segment in output file
  * '<-' previous interval
  * '->' next interval
  * 'R'/'T' change column number
  * 'Q'/'Esc' exit

## Options

```
  -h print help and exit
  -v print version and exit
  -o (FILE) save BED segment in that bed file (use key 'S')
  -D (int) cap depth to that value
  -B (FILE) list of path to indexed bam files
  -R (FILE) bed file of regions of interest
  -f (float) extend the regions by this factor
```


