#!/bin/tcsh -f
#
# build sara under win32
#
#######################################################################
# $Log: build_win32.sh,v $
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
# Revision 1.1  2004/03/23 18:54:44  doug
# working on win32
#
#######################################################################


# ---------------------------------------------------
set pushdsilent=1
alias make make -k
set path = ( $path /usr/bin/X11 )

# ---------------------------------------------------
# set up our environment
setenv TARGET_OS win32
setenv SETCMD setenv
setenv EQOP " "
setenv MIC_ROOT `pwd`
source $MIC_ROOT/support/set_env
source $MIC_ROOT/support/set_path

# ---------------------------------------------------
# Get list of packages to build 
source support/packages

# ---------------------------------------------------
# build the packages
@ haderror = 0
echo "Building package "
foreach package ($buildpackages)
   echo "   "$package
   cd $package
#   make depend
#   @ haderror = ($haderror || $status)
   make configure
#   @ haderror = ($haderror || $status)
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
