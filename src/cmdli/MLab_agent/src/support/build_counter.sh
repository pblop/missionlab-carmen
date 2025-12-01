#!/bin/sh
# Increment the build counter
# Author: Doug MacKenzie

# Get the current build number, default to 0 if no counter file exists
declare -i number=0

if test -e build_count; then \
   number=$[`cat build_count`]; \
fi

# Add one to the build
number=$[number + 1]

echo $number > build_count

# If a version file doesn't exist, create a default one with 0.0
if !(test -e build_version); then \
   echo 0.0 > build_version; \
fi
