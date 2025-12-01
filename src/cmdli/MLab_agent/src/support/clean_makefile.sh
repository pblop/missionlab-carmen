#!/bin/sh
#	Remove the dependency list from the bottom of a makefile
#
# Useage:
#	clean_makefile.sh makefile
#

if test $# -ne 1
then cat <<\xEOF
Useage:
        clean_makefile.sh makefile 
xEOF
exit 0
fi


# Make a tmp file
TMP1=/tmp/cleanup1
TMP2=/tmp/cleanup2
export TMP1
export TMP2

# sed commands to turn the old file into the new one
cat <<\xEOF > $TMP1
/\# DO NOT DELETE/,$c\
\# DO NOT DELETE\

xEOF
rm -f $TMP2
cp $1 $TMP2
sed -f $TMP1 < $TMP2 > $1

# clean up our fluff
rm -f $TMP1
rm -f $TMP2
