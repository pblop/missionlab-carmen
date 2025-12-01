#!/bin/tcsh -f
#
# checkout and build the sara cvs branch
#
#######################################################################
# $Log: build_branch.sh,v $
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
# Revision 1.8  2003/08/22 21:29:35  doug
# added matcon and libccl_gui
#
# Revision 1.7  2003/02/10 22:45:33  doug
# switch to objcoms instead of com_broadcast
#
# Revision 1.6  2002/11/01 16:08:16  doug
# renamed libraries
#
# Revision 1.5  2002/10/31 20:25:09  doug
# renamed libenv_objects to libdisplay2d and libdisplay3d
#
# Revision 1.4  2002/10/29 16:49:10  doug
# added libenv_objects
#
# Revision 1.3  2002/10/23 14:30:27  doug
# merging changes back from release
#
# Revision 1.1.2.3  2002/10/23 14:25:40  doug
# update to catch extra packages
#
# Revision 1.1.2.2  2002/10/23 13:56:39  doug
# merge changes to increment the build number
#
# Revision 1.1.2.1  2002/10/23 13:52:12  doug
# add a script to build the branch
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
# List of packages to build (in the order they will be built)
set buildpackages="libmic librc mic_rpcgen libobjthreads libobjcomms libipc_support libipc_robot2sim libipc_robot2con libfsa_io libFSAi libccl_io libasync_port libbase_c libdsp_objects libenv_objects libhardware_interface librobot_server_support librobot_server libccl_code libDTasker libtasks libccl_gui simserver controller opcon matcon"
set extrapackages="support sara_demos"

# ---------------------------------------------------
set branch=V0_1-Branch

echo "Building branch "$branch" in /tmp"
# ---------------------------------------------------
# get the the correct place
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
# check out the packages
echo "Checking out package "
foreach package ($extrapackages)
   echo "   "$package
   cvs checkout -r $branch $package
end
foreach package ($buildpackages)
   echo "   "$package
   cvs checkout -r $branch $package
end

# ---------------------------------------------------
# Increment the build count
cd support
./build_counter.sh
cvs ci -r $branch -m "update build number" build_count
cd ..

# ---------------------------------------------------
# set up our environment
setenv TARGET_OS linux
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
   make depend
   @ haderror = ($haderror || $status)
   make configure
   @ haderror = ($haderror || $status)
   make all
   @ haderror = ($haderror || $status)
   cd ..
end

# ---------------------------------------------------
# move back up
cd ..

# ---------------------------------------------------
# Error out if was not a good make
if( $haderror ) then
   echo "Error in build!"
   exit 1
endif

# ---------------------------------------------------
# delete the saved directory 
#if ( { test -e "./head~" } ) then
#   rm -rf head~
#endif
#mv head head~

# ---------------------------------------------------
# make the working directory current
#mv working head

#echo "Good build, making new head."
echo "Good build."
exit 0
