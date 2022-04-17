#!/usr/bin/bash
# compiles all the .tex files in this directory, including the subdirectories,
# writing the PDF output to the same location as its .tex source. see usage.

# directory this file resides in
REPO_HOME=$(realpath .)
# pdflatex command prefix we use
PDF_TEX="pdflatex -interaction=nonstopmode -halt-on-error -shell-escape"
# script usage
USAGE="usage: $0 [-h] [TEXFILE ...]

Compiles all .tex files in the repository to PDF.

pdflatex is invoked on each file with -halt-on-error and -shell-escape
options already preset. If arguments are passed to this script, assumed
to be names of .tex files, then only those files will be compiled.

PDF files will be compiled to same location as their corresponding .tex source.

optional arguments:
 -h, --help  show this usage
 TEXFILE     specified .tex file or files to compile, writing output to the
             directory TEXFILE is in. if omitted, all .tex files are compiled."

##
# Generate PDF file from a single specified .tex file.
#
# Arguments:
#   Name of the .tex file in this repo to compile
# Outputs:
#   Name of the resulting .pdf file if successful
#
compile_tex() {
    # save current directory so we can cd back
    CUR_DIR=$(pwd)
    # actual file path of the .tex file, path relative to CUR_DIR
    ACT_PATH=$(realpath $1)
    CUR_REL_PATH=$(realpath --relative-to=$CUR_DIR $1)
    # project name to send to pdflatex, bibtex; easier with no extension
    PROJ_NAME=$(basename $1)
    PROJ_NAME=${PROJ_NAME%%.tex}
    # still best to cd to directory of .tex file to do compilation so that
    # relative image and bibliography includes work correctly
    cd $(dirname $ACT_PATH)
    # standard pdflatex -> bibtex -> pdflatex -> pdflatex
    $PDF_TEX $PROJ_NAME > /dev/null
    # ignore so set -e works. if there is no citation, bibdata, or bibstyle
    # commands, BibTeX exists nonzero, although this error is nonfatal.
    bibtex $PROJ_NAME > /dev/null || true
    $PDF_TEX $PROJ_NAME > /dev/null
    $PDF_TEX $PROJ_NAME > /dev/null
    cd $CUR_DIR
    echo "compiled $(echo $CUR_REL_PATH | sed s/.tex/.pdf/g)"
}

##
# Recursively compile all .tex files in the repo to their locations in PDF_DIR.
#
# Arguments:
#   Name of the current directory to start recursively compiling
#       Optional. If not provided, REPO_HOME is used. If you give it multiple
#       directories, only the first one will be used.
# Outputs:
#   Names of the resulting .pdf files generated (from compile_tex)
#
compile_all_tex() {
    if [ $# -eq 0 ]
    then
        CUR_DIR=$REPO_HOME
    elif [ $# -ge 1 ]
    then
        CUR_DIR=$(realpath $1)
    fi
    for FILE_OR_DIR in $CUR_DIR/*
    do
        if [ -d $FILE_OR_DIR ]
        then
            compile_all_tex $FILE_OR_DIR
        elif [[ $FILE_OR_DIR == *.tex ]]
        then
            compile_tex $FILE_OR_DIR  
        fi
    done
}

##
# Main function for the script.
#
# If no arguments are provided, all TeX files in repo are compiled to PDF.
#
# Arguments:
#   -h, --help or list of individual TeX files to compile
#
main() {
    # if no arguments, recursively compile all files in all subdirectories
    if [ $# -eq 0 ]
    then
        compile_all_tex
    # case for help output
    elif [ $# -eq 1 ] && { [ $1 = "-h" ] || [ $1 = "--help" ]; }
    then
        # need double quote to preserve the spacing
        echo "$USAGE"
    # otherwise, just assume they are all files to compile
    else
        for MAYBE_TEX in "$@"
        do
            compile_tex $MAYBE_TEX
        done
    fi
}

set -e
main "$@"
