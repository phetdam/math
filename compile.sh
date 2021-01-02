#!/usr/bin/bash
# compiles all the .tex files in this directory and writes the output to output.
# all other arguments passed are sent directly to pdflatex.

# script usage
USAGE="usage: aa [-h] [TEXFILE]

Compiles all .tex files in the directory, writing PDF output to ./output.

pdflatex is invoked on each file with -shell-escape and -out-directory options
already preset. If an argument is passed to this script, assumed to be a .tex
file, then only that file name will be compiled with output written to ./output.

optional arguments:
 -h, --help  show this usage
 TEXFILE     specified .tex file to compile, writing output to ./output"
# pdflatex compile command. for some reason, -output-directory is ignored.
# therefore, i used -jobname to modify the output directory, which works for me.
TEX_COMPILE="pdflatex -halt-on-error -shell-escape"

# if no arguments, then compile all files
if [[ $# == 0 ]]
then
    for INFILE in ./*.tex
    do
        # we could use bash string manipulation but i wanted to make this POSIX
        $TEX_COMPILE -jobname="$(echo ./output/$INFILE | sed s/.tex//g)" $INFILE
    done
# else if 1 argument
elif [[ $# == 1 ]]
then
    # if help argument, then print usage
    if [ $1 = "-h" ] || [ $1 = "--help" ]
    then
        # need double quote to preserve the spacing
        echo "$USAGE"
    fi
    # else treat as .tex file and send to pdflatex
    $TEX_COMPILE -jobname="$(echo ./output/$1 | sed s/.tex//g)" $1
fi