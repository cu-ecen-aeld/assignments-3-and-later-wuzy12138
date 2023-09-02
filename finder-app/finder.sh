#!/bin/sh

filesdir=$1
searchstr=$2

if [ ! -n "${filesdir}" ] || [ ! -n "${searchstr}" ]
then
    echo "There should be two params"
    exit 1
fi

if [ ! -d "${filesdir}" ]
then 
    echo "The first argument does not represent a directory on the filesystem"
    exit 1
fi

numberOfFile=$(find -L $filesdir -type f | wc -l)
numberOfLine=$(grep -Rn $searchstr $filesdir| wc -l)

echo "The number of files are ${numberOfFile} and the number of matching lines are ${numberOfLine}."

