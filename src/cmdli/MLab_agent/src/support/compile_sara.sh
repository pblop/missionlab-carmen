#!/bin/tcsh -f
#
# build the sara head
#
#######################################################################
# $Log: compile_sara.sh,v $
# Revision 1.1.1.1  2008/07/14 16:44:17  endo
# MAST Project (based on MissionLab-MINOS-20071018.tar.gz)
#
# Revision 1.1.1.1  2006/07/20 17:17:46  endo
# MINOS Project (based on MissionLab-7.0.20060712.tar.gz)
#
# Revision 1.1.1.1  2006/07/12 13:37:55  endo
# MissionLab 7.0
#
# Revision 1.1  2006/07/01 00:37:43  endo
# CMDLi from MARS 2020 migrated into AO-FNC repository.
#
# Revision 1.1.1.1  2006/06/29 20:42:07  endo
# cmdli local repository.
#
# Revision 1.3  2004/04/15 19:12:45  doug
# building DLL using new flags
#
# Revision 1.2  2003/08/25 13:53:46  doug
# use a single list of packages
#
# Revision 1.1  2003/04/11 18:47:23  doug
# just do a compile
#
# Revision 1.3  2003/04/07 12:44:36  doug
# moving to RedHat 9
#
# Revision 1.2  2003/03/03 18:42:09  doug
# fix ending bug
#
# Revision 1.1  2003/02/14 22:05:00  doug
# added a build-in-place command
#
#######################################################################


# ---------------------------------------------------
# Get list of packages to build 
source support/packages

# ---------------------------------------------------
set pushdsilent=1
alias make make -k
set path = ( $path /usr/bin/X11 )

# ---------------------------------------------------
# set up our environment
#setenv TARGET_OS linux
setenv SETCMD setenv
setenv EQOP " "
setenv MIC_ROOT `pwd`
source $MIC_ROOT/support/set_env
source $MIC_ROOT/support/set_path

# ---------------------------------------------------
# build the packages
@ haderror = 0
echo "Building package "
foreach package ($buildpackages)
   echo "   "$package
   cd $package
   make all
   @ haderror = ($haderror || $status)
   cd ..
end

# ---------------------------------------------------
# Error out if was not a good make
if( $haderror ) then
   echo "Error in build!"
   exit 1
endif

echo "Good build."
exit 0
