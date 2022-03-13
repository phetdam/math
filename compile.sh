#!/usr/bin/bash
# compiles all the .tex files in this directory, including the subdirectories,
# and writes the output to PDF_DIR, specified in this script, duplicating the
# existing file structure of this repo's .tex files. see usage for details.

# directory this file resides in
REPO_HOME=$(realpath .)
# relative and absolute output directory
REL_PDF_DIR="pdf"
PDF_DIR="$REPO_HOME/$REL_PDF_DIR"
# script usage
USAGE="usage: $0 [-h] [TEXFILE ...]

Compiles all .tex files in the repository, writing PDF output to $REL_PDF_DIR.

pdflatex is invoked on each file with -halt-on-error and -shell-escape
options already preset. If arguments are passed to this script, assumed
to be names of .tex files, then only those files will be compiled.

note that any file with path relative to the repo home, ex.
my/path/file.tex, will be written to $REL_PDF_DIR/my/path/file.tex.

If $REL_PDF_DIR and child dirs with .tex files do not exist, they will be
created. stdout output goes to /dev/null, but stderr output is displayed.

optional arguments:
 -h, --help  show this usage
 TEXFILE     specified .tex file or files to compile, writing output to
             $REL_PDF_DIR. if omitted, all .tex files in repo are compiled."

##
# Recursively creates empty directories in PDF_DIR mirroring repo structure.
#
# Arguments:
#   Name of current directory to start generate subdirectories for.
#       Optional. If not provided, uses REPO_HOME. If given multiple directory
#       names, will only use the first one specified.
# Outputs:
#   Names of any empty directories created.
create_output_dirs() {
    if [ $# -eq 0 ]
    then
        CUR_DIR=$REPO_HOME
    elif [ $# -ge 1 ]
    then
        # easier to work with absolute paths
        CUR_DIR=$(realpath $1)
    fi
    # only create $PDF_DIR if CUR_DIR is REPO_HOME
    if [ $CUR_DIR = $REPO_HOME ] && [ ! -d $PDF_DIR ]
    then
        mkdir $PDF_DIR
        echo "created output directory $PDF_DIR"
    fi
    for MAYBE_DIR in $CUR_DIR/*
    do
        MAYBE_NEW_DIR="$PDF_DIR/$(realpath --relative-to=$REPO_HOME $MAYBE_DIR)"
        if  [ -d $MAYBE_DIR ] && \
            # if there aren't any .tex files in the dir, we won't create it
            [ $(echo $MAYBE_DIR/*.tex | wc -w) -gt 0 ] && \
            [ $MAYBE_DIR != $PDF_DIR ] && \
            [ $(realpath --relative-to=$CUR_DIR $MAYBE_DIR) != "images" ] && \
            [ ! -d $MAYBE_NEW_DIR ]
        then
            mkdir $MAYBE_NEW_DIR
            echo "created new directory $MAYBE_NEW_DIR"
            create_output_dirs $MAYBE_DIR
        fi
    done
}

##
# Compiles a single specified .tex file to its corresponding output dir.
#
# Arguments:
#   Name of the .tex file in this repo to compile. Won't work if not in repo.
# Outputs:
#   Name of the resulting .pdf file if successful.
compile_tex() {
    # needs to be relative to repo directory
    REL_PATH=$(realpath --relative-to=$REPO_HOME $1)
    ACT_PATH=$(realpath $1)
    pdflatex -halt-on-error -shell-escape \
        -output-directory=$PDF_DIR/$(dirname $REL_PATH) $ACT_PATH > /dev/null
    echo "compiled $(echo $ACT_PATH | sed s/.tex/.pdf/g)"
}

##
# Recursively compile all .tex files in the repo to their locations in PDF_DIR.
#
# Arguments:
#   Name of the current directory to start recursively compiling.
#       Optional. If not provided, REPO_HOME is used. If you give it multiple
#       directories, only the first one will be used.
# Outputs:
#   Names of the resulting .pdf files generated (from compile_tex).
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
main() {
    # if output and other subdirectories don't exist, create them
    create_output_dirs
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
