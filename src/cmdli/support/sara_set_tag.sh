#!/bin/tcsh -f
#
# checkin the sara cvs head
#
#######################################################################
# $Log: sara_set_tag.sh,v $
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
# Revision 1.1  2004/10/30 00:35:23  doug
# add ability to set tags
#
#######################################################################

if ( $#argv != 1 ) then
    echo "usage:  sara_set_tag "'"'tag'"'
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
echo "Setting tag in extra package "
foreach package ($extrapackages)
   echo "   "$package
   cd $package
   cvs -q tag "$argv[1]"
   cd ..
end
echo "Setting tag in linux package "
foreach package ($buildpackages)
   echo "   "$package
   cd $package
   cvs -q tag "$argv[1]"
   cd ..
end


setenv TARGET_OS win32
source support/packages
echo "Setting tag in win32 package "
#foreach package ($buildpackages)
#   echo "   "$package
#   cd $package
#   cvs commit -m "$argv[1]"
#   cd ..
#end
   echo "   UPenn_DLL"
   cd UPenn_DLL
   cvs -q tag "$argv[1]"
   cd ..
