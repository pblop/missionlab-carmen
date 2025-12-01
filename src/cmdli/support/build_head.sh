#!/bin/tcsh -f
#
# checkout and build the sara cvs head
#
#######################################################################
# $Log: build_head.sh,v $
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
# Revision 1.33  2004/11/04 19:39:05  doug
# dump the win32 builds
#
# Revision 1.32  2004/10/29 22:36:00  doug
# getting win32 to build again
#
# Revision 1.31  2004/08/10 13:11:07  doug
# do not build win32 as part of nightly builds
#
# Revision 1.30  2004/06/17 17:18:51  doug
# build both linux and win32
#
# Revision 1.29  2004/03/25 13:55:32  doug
# need to set TARGET earlier
#
# Revision 1.28  2004/03/18 13:35:26  doug
# move init of CVSROOT before first cvs call
#
# Revision 1.27  2004/02/28 18:23:50  doug
# need to update the support directory
#
# Revision 1.26  2003/08/25 13:44:00  doug
# added packages
#
# Revision 1.25  2003/08/25 13:38:05  doug
# added packages
#
# Revision 1.24  2003/02/10 22:45:33  doug
# switch to objcoms instead of com_broadcast
#
# Revision 1.23  2002/11/15 21:43:09  doug
# added satellite_photos to build script
#
# Revision 1.22  2002/11/06 19:28:47  doug
# reorder build of libdsp_objects and libenv_objects
#
# Revision 1.21  2002/11/01 16:08:16  doug
# renamed libraries
#
# Revision 1.20  2002/10/31 20:25:09  doug
# renamed libenv_objects to libdisplay2d and libdisplay3d
#
# Revision 1.19  2002/10/29 16:49:10  doug
# added libenv_objects
#
# Revision 1.18  2002/10/23 18:29:58  builds
# removed invalid code copied from build_branch
#
# Revision 1.17  2002/10/23 14:30:27  doug
# merging changes back from release
#
# Revision 1.15.2.1  2002/10/23 14:25:40  doug
# update to catch extra packages
#
# Revision 1.15  2002/10/10 12:58:05  doug
# need to set path so can find makedepend
#
# Revision 1.14  2002/10/09 13:26:12  doug
# update the build_head script
#
# Revision 1.13  2002/10/07 17:19:49  doug
# try to use the exported path so gdb can find the source files
#
# Revision 1.12  2002/10/04 21:58:26  doug
# adding crontab
#
# Revision 1.11  2002/10/04 21:04:46  doug
# cleanup
#
# Revision 1.10  2002/10/04 20:21:45  doug
# removed debug code
#
#
#######################################################################

# ---------------------------------------------------
set pushdsilent=1
alias cvs cvs -Q -d :pserver:builds@oak:/cvsroot
alias make make -k
set path = ( $path /usr/bin/X11 )

# ---------------------------------------------------
# get the the correct place
cd /shares/builds/sara

# ---------------------------------------------------
# make a working directory
if ( { test -e ./working } ) then
   rm -rf working
endif
mkdir working

# ---------------------------------------------------
# move into it
cd ./working

# ---------------------------------------------------
# get the support packages
echo "Checking out support packages"
cvs checkout support
setenv TARGET_OS linux
source support/packages
foreach package ($extrapackages)
   echo "   "$package
   cvs checkout $package
end

# ---------------------------------------------------
# check out the packages

echo "Checking out linux package "
setenv TARGET_OS linux
source support/packages
foreach package ($buildpackages)
   echo "   "$package
   cvs checkout $package
end

echo "Checking out win32 package "
setenv TARGET_OS win32
source support/packages
foreach package ($buildpackages)
   echo "   "$package
   cvs checkout $package
end

# ---------------------------------------------------
# Increment the build count
cd support
./build_counter.sh
cvs ci -m "update build number" build_count
cd ..

# ---------------------------------------------------
# set up our environment
setenv SETCMD setenv
setenv EQOP " "
setenv MIC_ROOT `pwd`
source $MIC_ROOT/support/set_env
source $MIC_ROOT/support/set_path

# ---------------------------------------------------
# build the packages
@ haderror = 0
echo "Building linux package "
setenv TARGET_OS linux
source support/packages
foreach package ($buildpackages)
   echo "   "$package
   cd $package
   make depend
   @ haderror = ($haderror || $status)
   make configure
   @ haderror = ($haderror || $status)
   make all
   @ haderror = ($haderror || $status)
   cd ..
end

#echo "Building win32 package "
#setenv TARGET_OS win32
#source support/packages
#foreach package ($buildpackages)
#   echo "   "$package
#   cd $package
##   make depend
##   @ haderror = ($haderror || $status)
#   make configure
#   @ haderror = ($haderror || $status)
#   make all
#   @ haderror = ($haderror || $status)
#   cd ..
#end

# ---------------------------------------------------
# move back up
cd ..

# ---------------------------------------------------
# Error out if was not a good make
if( $haderror ) then
   echo "Error in build, leaving the stale head!"
   exit 1
endif

# ---------------------------------------------------
# delete the saved directory 
if ( { test -e "./head~" } ) then
   rm -rf head~
endif
mv head head~

# ---------------------------------------------------
# make the working directory current
mv working head

echo "Good build, making new head."
exit 0
