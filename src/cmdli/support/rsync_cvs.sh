#!/bin/tcsh -f

# ---------------------------------------------------
# List of packages to mirror 
set packages="libmic librc mic_rpcgen libobjthreads libobjcomms libipc_support libipc_robot2sim libipc_robot2con libfsa_io libFSAi libccl_io libdsp_objects libenv_objects libhardware_interface librobot_server_support librobot_server libccl_code libDTasker simserver controller opcon"

# ---------------------------------------------------
# move to the cvsroot
cd /exports/cvsroot

# ---------------------------------------------------
# Push all changes to the mirror
foreach package ($packages)
   rsync -a --delete -e ssh $package/ cvs@mic4.mobile-intelligence.com:/home/cvs/cvsroot/$package
end

# ---------------------------------------------------
