![works on my machine](https://img.shields.io/badge/works-on%20my%20machine-green.png)

![Last commit](https://img.shields.io/github/last-commit/lindenb/x11hts.png)

# x11hts

X11 related utilities for hts

## Author

Pierre Lindenbaum PhD / @yokofakun 2019

## Screenshot

![https://twitter.com/yokofakun/status/1111224539307016193](https://pbs.twimg.com/media/D2vcmoOWoAADKe8.jpg)

## Compilation

dependencies:

  * **X11/dev** libraries 
  * **htslib** :  https://github.com/samtools/htslib 

compile using make, specifying the path to HTSLIB

```
 $ make HTSLIB=${HOME}/packages/htslib
 ```

should generate a program named `x11hts`

sometimes you may get this message:

```
x11hts: error while loading shared libraries: libhts.so.1: cannot open shared object file: No such file or directory
```

and you'll have to set the e `LD_LIBRARY_PATH`

```
export LD_LIBRARY_PATH=/home/lindenb/packages/htslib
```



# CNV
  Displays Bam coverage

## Example

```
./x11hts cnv -B bam.list -f 0.3 -R input.bed
```


## Options

run the following command to display the options & keys:

```
x11hts cnv -h
```


