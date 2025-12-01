#!/bin/tcsh -f
#
# checkout and build the sara cvs head
#
#######################################################################
# $Log: checkout_sara.sh,v $
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
# Revision 1.7  2004/10/30 00:29:53  doug
# Sending out to MARS2020
#
# Revision 1.6  2003/08/25 13:53:46  doug
# use a single list of packages
#
# Revision 1.5  2003/08/22 21:29:35  doug
# added matcon and libccl_gui
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

# ---------------------------------------------------
# Get list of packages 
source support/packages

# ---------------------------------------------------
set pushdsilent=1
alias cvs cvs -Q -d :pserver:$user@oak:/cvsroot
alias make make -k
set path = ( $path /usr/bin/X11 )

# ---------------------------------------------------
# check out the packages
echo "Checking out package "
foreach package ($extrapackages)
   echo "   "$package
   cvs checkout $package
end
foreach package ($buildpackages)
   echo "   "$package
   cvs checkout $package
end

cvs checkout UPenn_DLL

