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

if $OUT_DIR does not exist, it will first be created. output printed to stdout
will be sent to /dev/null but output printed to stderr will still be displayed.

optional arguments:
 -h, --help  show this usage
 TEXFILE     specified .tex file to compile, writing output to $OUT_DIR. if
             omitted, then all files in the current directory are compiled."
# pdflatex compile command. for some reason, -output-directory is ignored.
# therefore, i used -jobname to modify the output directory, which works for me.
PDF_TEX="pdflatex -halt-on-error -shell-escape"

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
        # note redirect to /dev/null doesn't include 2>&1 so output from stderr
        # will still be shown. echo + sed used to replace .tex with empty string
        $PDF_TEX -jobname="$(echo $OUT_DIR/$INFILE | sed s/.tex//g)" $INFILE \
            > /dev/null
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
        $PDF_TEX -jobname="$(echo $OUT_DIR/$1 | sed s/.tex//g)" $1 > /dev/null
    fi
# else too many arguments
else
    echo "$0: too many arguments. try $0 --help for usage"
fi