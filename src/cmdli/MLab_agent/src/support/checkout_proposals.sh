#!/bin/tcsh -f
#
# checkout all proposals
#
#######################################################################
# $Log: checkout_proposals.sh,v $
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
# Revision 1.2  2004/01/09 19:56:33  doug
# use env variable to find support directory
#
# Revision 1.1  2004/01/09 19:53:24  doug
# added support for proposals
#
#######################################################################

# ---------------------------------------------------
# Get list of packages 
source $SUPPORT_DIR/proposals

# ---------------------------------------------------
set pushdsilent=1
alias cvs cvs -Q -d :pserver:$user@oak:/cvsroot
alias make make -k
set path = ( $path /usr/bin/X11 )

# ---------------------------------------------------
# check out the packages
echo "Checking out package "
foreach package ($proposals)
   echo "   "$package
   cvs checkout $package
end

