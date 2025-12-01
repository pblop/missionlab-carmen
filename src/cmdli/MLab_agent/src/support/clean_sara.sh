#!/bin/tcsh -f
#
# build the sara head
#
#######################################################################
# $Log: clean_sara.sh,v $
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
# Revision 1.2  2006/06/30 23:14:47  endo
# No more Windows clean.
#
# Revision 1.1.1.1  2006/06/29 20:42:07  endo
# cmdli local repository.
#
# Revision 1.5  2004/03/26 22:40:14  doug
# making progress on win32 build
#
# Revision 1.4  2004/03/26 17:02:51  doug
# switched to using make.target
#
# Revision 1.3  2004/02/28 16:35:15  doug
# getting cmdli to work in sara
#
# Revision 1.2  2003/08/29 21:40:41  doug
# moved ccl procedures to Procedure instead of ProcedureBase
#
# Revision 1.1  2003/02/26 14:57:20  doug
# controller now runs with objcomms
#
# Revision 1.1  2003/02/14 22:05:00  doug
# added a build-in-place command
#
#######################################################################


# ---------------------------------------------------
# setup
setenv SETCMD setenv
setenv EQOP " "
setenv MIC_ROOT `pwd`
source $MIC_ROOT/support/set_env
source $MIC_ROOT/support/set_path

# ---------------------------------------------------
set pushdsilent=1
alias make make -k
set path = ( $path /usr/bin/X11 )

# ---------------------------------------------------
# do it once for linux
setenv TARGET_OS linux
source support/packages
echo "Cleaning linux package "
foreach package ($buildpackages)
   echo "   "$package
   cd $package
   make veryclean
   cd ..
end

# ---------------------------------------------------
# do it again for win32
#setenv TARGET_OS win32
#source support/packages
#echo "Cleaning win32 package "
#foreach package ($buildpackages)
#   echo "   "$package
#   cd $package
#   make veryclean
#   cd ..
#end
#
# ---------------------------------------------------
echo "Done."
exit 0
