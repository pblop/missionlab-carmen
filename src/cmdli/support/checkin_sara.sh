#!/bin/tcsh -f
#
# checkin the sara cvs head
#
#######################################################################
# $Log: checkin_sara.sh,v $
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
# Revision 1.18  2004/04/05 22:20:07  doug
# fix a comment typo
#
# Revision 1.17  2004/03/26 17:06:27  doug
# switched to using make.target
#
# Revision 1.16  2003/08/25 13:53:46  doug
# use a single list of packages
#
# Revision 1.15  2003/08/22 21:29:35  doug
# added matcon and libccl_gui
#
# Revision 1.14  2003/04/21 17:43:25  doug
# working on adding tcp support to objcomms to handle large data streams point to point
#
# Revision 1.13  2003/04/21 17:42:29  doug
# working on adding tcp support to objcomms to handle large data streams point to point
#
# Revision 1.12  2003/04/21 17:41:54  doug
# working on adding tcp support to objcomms to handle large data streams point to point
#
# Revision 1.11  2003/04/21 17:40:27  doug
# working on adding tcp support to objcomms to handle large data streams point to point
#
# Revision 1.10  2003/04/21 17:23:06  doug
# finally fixed to accept a command line comment
#
# Revision 1.9  2003/04/11 18:46:53  doug
# added export capabilities
#
# Revision 1.8  2003/04/07 12:44:36  doug
# moving to RedHat 9
#
# Revision 1.7  2003/04/02 22:11:47  doug
# snapshot
#
# Revision 1.6  2003/03/27 22:38:19  doug
# moved libccl_code to the sara namespace
#
# Revision 1.5  2003/03/15 20:40:51  doug
# moved libccl_code to the sara namespace
#
# Revision 1.4  2003/03/15 19:46:55  doug
# update
#
# Revision 1.3  2003/03/10 16:31:33  doug
# FittedCurve is closer
#
# Revision 1.2  2003/02/27 22:42:59  doug
# seems to work
#
# Revision 1.1  2003/02/26 22:16:18  doug
# integrated with opcon
#
# Revision 1.4  2003/02/10 22:45:33  doug
# switch to objcoms instead of com_broadcast
#
# Revision 1.3  2003/01/24 22:41:17  doug
# added libobjcomms to checkout_sara
#
# Revision 1.2  2002/11/23 22:26:25  doug
# added checkout script
#
# Revision 1.1  2002/11/23 22:24:29  doug
# added checkout script
#
#######################################################################

if ( $#argv != 1 ) then
    echo "usage:  checkin_sara "'"'comment'"'
    exit 1
endif

# ---------------------------------------------------
set pushdsilent=1
alias cvs cvs -Q -d :pserver:$user@oak:/cvsroot
alias make make -k
set path = ( $path /usr/bin/X11 )

# ---------------------------------------------------
# Do it once for linux

# Get list of packages 
setenv TARGET_OS linux
source support/packages
echo "Checking in extra package "
foreach package ($extrapackages)
   echo "   "$package
   cd $package
   cvs commit -m "$argv[1]"
   cd ..
end
echo "Checking in linux package "
foreach package ($buildpackages)
   echo "   "$package
   cd $package
   cvs commit -m "$argv[1]"
   cd ..
end


setenv TARGET_OS win32
source support/packages
echo "checking in win32 package "
foreach package ($buildpackages)
   echo "   "$package
   cd $package
   cvs commit -m "$argv[1]"
   cd ..
end

