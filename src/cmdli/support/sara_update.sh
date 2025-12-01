#!/bin/tcsh -f
#
# update the local copies with the latest info from the repository
#
#######################################################################
# $Log: sara_update.sh,v $
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
# Revision 1.1  2003/04/22 23:52:06  doug
# update local version from repository
#
#######################################################################

# ---------------------------------------------------
# List of packages to build (in the order they will be built)
set buildpackages="libmic librc mic_rpcgen libobjthreads libobjcomms libipc_support libipc_robot2sim libipc_robot2con libfsa_io libFSAi libccl_io libasync_port libbase_c libenv_objects libdsp_objects libhardware_interface librobot_server_support librobot_server libccl_code libDTasker libtasks simserver controller opcon"
set extrapackages="support sara_demos satellite_photos"

# ---------------------------------------------------
alias cvs cvs -Q -d :pserver:$user@oak:/cvsroot
alias make make -k
set path = ( $path /usr/bin/X11 )

# ---------------------------------------------------
# check out the packages
echo "Checking out package "
foreach package ($extrapackages)
   echo "   "$package
   cvs update $package
end
foreach package ($buildpackages)
   echo "   "$package
   cvs update $package
end

