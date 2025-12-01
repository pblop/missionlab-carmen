#!/bin/tcsh -f
#
# branch the sara cvs source tree
#
#######################################################################
# $Log: branch_sara.sh,v $
# Revision 1.1.1.1  2008/07/14 16:44:17  endo
# MAST Project (based on MissionLab-MINOS-20071018.tar.gz)
#
# Revision 1.1.1.1  2006/07/20 17:17:46  endo
# MINOS Project (based on MissionLab-7.0.20060712.tar.gz)
#
# Revision 1.1.1.1  2006/07/12 13:37:55  endo
# MissionLab 7.0
#
# Revision 1.1  2006/07/01 00:37:42  endo
# CMDLi from MARS 2020 migrated into AO-FNC repository.
#
# Revision 1.1.1.1  2006/06/29 20:42:07  endo
# cmdli local repository.
#
# Revision 1.6  2003/08/29 21:40:41  doug
# moved ccl procedures to Procedure instead of ProcedureBase
#
# Revision 1.5  2003/02/10 22:45:33  doug
# switch to objcoms instead of com_broadcast
#
# Revision 1.4  2002/11/01 16:08:16  doug
# renamed libraries
#
# Revision 1.3  2002/10/31 20:25:09  doug
# renamed libenv_objects to libdisplay2d and libdisplay3d
#
# Revision 1.2  2002/10/29 16:49:10  doug
# added libenv_objects
#
# Revision 1.1  2002/10/22 20:40:28  doug
# adding branch and release capabilities
#
#######################################################################

if ( $#argv != 1 ) then
    echo "usage:  $argv[0] n_n"
    echo "        where n_n is the version number, n.n"
    exit 1
endif

set number=$argv[1]
set tag="RootOf-V"$number
set branch="V"$number"-Branch"

# ---------------------------------------------------
# Get list of packages to build 
source support/packages

# ---------------------------------------------------
# move to the correct place
#cd /shares/builds/sara
cd /tmp

# ---------------------------------------------------
# make a working directory
if ( { test -e ./working } ) then
   rm -rf working
endif
mkdir working

# ---------------------------------------------------
set pushdsilent=1
#alias cvs cvs -Q -d :pserver:builds@oak:/cvsroot
alias cvs cvs -Q 
alias make make -k
set path = ( $path /usr/bin/X11 )

# ---------------------------------------------------
# move into it
cd ./working

# ---------------------------------------------------
# check out the packages, set a tag at the branch point,
# and make the branch
foreach package ($buildpackages $extrapackages)
   echo "Checking out package "$package
   cvs checkout $package

   cd $package
   echo "Setting tag "$tag" at branch point and create branch "$branch
   cvs tag $tag
   cvs tag -b $branch
   cd ..
end

# ---------------------------------------------------
# move back to where we started
cd ..

# Remove the working directory
if ( { test -e ./working } ) then
   rm -rf working
endif

# ---------------------------------------------------
# done
exit 0
