#!/bin/tcsh -f
#
# make a release from the head of an existing branch in the sara cvs source tree
#
#######################################################################
# $Log: release_sara.sh,v $
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
# Revision 1.5  2003/02/10 22:45:33  doug
# switch to objcoms instead of com_broadcast
#
# Revision 1.4  2002/11/01 16:08:16  doug
# renamed libraries
#
# Revision 1.3  2002/10/31 20:25:09  doug
# renamed libenv_objects to libdisplay2d and libdisplay3d
#
# Revision 1.2  2002/10/29 16:49:10  doug
# added libenv_objects
#
# Revision 1.1  2002/10/22 20:40:28  doug
# adding branch and release capabilities
#
#######################################################################

if ( $#argv != 1 && $#argv != 2 ) then
    echo "usage:  $argv[0] n_n [p]"
    echo "        where n_n is the branch version number, n.n"
    echo "        and p is an optional point release number or string"
    echo "        The release tag will be called Rn_n_p"
    exit 1
endif

set branch="V"$argv[1]"-Branch"
if ( $#argv == 2 ) then
   set release="R"$argv[1]"_"$argv[2]
else
   set release="R"$argv[1]
endif

# ---------------------------------------------------
# List of packages in the release 
set packages="support libmic librc mic_rpcgen libobjthreads libobjcomms libipc_support libipc_robot2sim libipc_robot2con libfsa_io libFSAi libccl_io libasync_port libbase_c libdsp_objects libenv_objects libhardware_interface librobot_server_support librobot_server libccl_code libDTasker libtasks simserver controller opcon sara_demos"

# ---------------------------------------------------
# move to the correct place
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
# check out the packages and set a tag a the release point
foreach package ($packages)
   echo "Checking out package "$package" from branch "$branch
   cvs checkout -r $branch $package

   echo "Setting tag "$release
   cd $package
echo   cvs tag $release
   cd ..
end

# ---------------------------------------------------
# move back to where we started
cd ..

# Remove the working directory
#if ( { test -e ./working } ) then
#   rm -rf working
#endif

# ---------------------------------------------------
# done
exit 0


















# make sure we got one parm
if test $# -ne 1
then cat <<\xEOF
Useage:
	make_release number
xEOF
exit 0
fi

if test -e development/common 
then
#correct the development build version
echo $1 > development/common/build_version

#reset the development build number
rm development/common/build_count
fi

#copy the directory over
cp -R development release_$1

#rebuild it insitu
cd release_$1
make veryclean
make depend
make all
make clean
cd ..

#set to read only
chmod -R ogu-w release_$1




#done
