#!/usr/bin/bash
# compiles all the .tex files in this directory and writes the output to
# OUT_DIR, specified in this script. see usage for details.

# output directory
OUT_DIR="./output"
# script usage
USAGE="usage: $0 [-h] [TEXFILE]

Compiles all .tex files in the directory, writing PDF output to $OUT_DIR.

pdflatex is invoked on each file with -shell-escape and -out-directory options
already preset. If an argument is passed to this script, assumed to be a .tex
file, then only that file name will be compiled with output written to $OUT_DIR.

if $OUT_DIR does not exist, it will first be created.

optional arguments:
 -h, --help  show this usage
 TEXFILE     specified .tex file to compile, writing output to $OUT_DIR. if
             omitted, then all files in the current directory are compiled."
# pdflatex compile command. for some reason, -output-directory is ignored.
# therefore, i used -jobname to modify the output directory, which works for me.
TEX_COMPILE="pdflatex -halt-on-error -shell-escape"

# if output doesn't exist, then create it
if [ ! -d $OUTPUT_DIR ]
then
    mkdir $OUTPUT_DIR
fi
# if no arguments, then compile all files
if [[ $# == 0 ]]
then
    for INFILE in ./*.tex
    do
        # we could use bash string manipulation but i wanted to make this POSIX
        $TEX_COMPILE -jobname="$(echo $OUT_DIR/$INFILE | sed s/.tex//g)" $INFILE
    done
# else if 1 argument
elif [[ $# == 1 ]]
then
    # if help argument, then print usage
    if [ $1 = "-h" ] || [ $1 = "--help" ]
    then
        # need double quote to preserve the spacing
        echo "$USAGE"
    # else treat as .tex file and send to pdflatex
    else
        $TEX_COMPILE -jobname="$(echo ./$OUT_DIR/$1 | sed s/.tex//g)" $1
    fi
# else too many arguments
else
    echo "$0: too many arguments. try $0 --help for usage"
fi