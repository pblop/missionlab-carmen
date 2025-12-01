
# Make backup CDL files from the editor (true or false).
backup_files = True

# Show the values of the slider bars instead of the symbolic names
ShowSliderValues = True

# Set the capabilities of the user.
#   Need Execute in order to run a configuration  
#   Need Modify in order to be able to modify parameters in a configuration
#   Need Edit in order to be able to create or modify configurations
#   Need Create in order to be able to create new components
#   Need Library in order to be able to modify library components
#   Need RealRobots in order to be able to run real robots
 
user_privileges = Execute, Modify, Edit, Create, Library, RealRobots


# Select which primitives are shown to the user
# List architectures here to restrict names to only those names occuring
# in all of the listed architectures.  The default is to show all names.
# Architectures:
#    AuRA
#    UGV
#    SWARM
#
#restrict_names = AuRA

# List of comma seperated directories and root names of the 
# CDL description libraries to load.
# Will try to load xxx.gen, xxx.AuRA, and xxx.UGV 
CDL_libraries = /home/doug/mobile_intelligence/baselines/development/cdl_primitives/cdl_primitives,
                /home/doug/mobile_intelligence/baselines/development/cdl_assemblages/cdl_assemblages


# Optional: Configuration to load as the empty config.
#DefaultConfiguration = /users/r/robot/mission/lib/FSA.cdl

# Where to find the map overlays
MapOverlays = /users/r/robot/mission/overlays

# Directory to write the event logs.
# None are written if this is empty.
# EventLogDir = .

# Extra robots to add to the run.
#ExtraRobots = "Enemy wanderRobot1 red (robot_type = MRV2, run_type= SIMULATION)"

# Offsets to start robots at.
#RobotStart_dx = 10
#RobotStart_dy = 5

# *********** Select compilers ********************
AuRA_CNL = cnl
SWARM_CNL = mic_cnl
AuRA_CC = gcc
SWARM_CC = dos-gcc

# *********** CNL Architecture configuration ********************

# Directories to look in for link libraries.   
# Seperate each path with a colon.  Newlines are allowed after the colon.
lib_paths = /users/r/robot/mission/lib

# List of comma seperated directories and root names of the 
# CNL source files and libraries to load.
# Will try to load libxxx.a and include xxx.inc as the cnl header file
# The editor will look for yyy.cnl in these locations for each extern agent
CNL_libraries = /home/doug/mobile_intelligence/baselines/development/lib/procs

# directories to look in for CNL source files to display in the editor.
CNL_sources = /home/doug/mobile_intelligence/baselines/development/cnl_procs

# Include directories
Includes = /home/doug/mobile_intelligence/baselines/development/include

# cflags parm passed to C++ compiler
cflags = -g,
	 -Xlinker,
	 -Map,
	 -Xlinker,
	 cfg.map

# ldflags parm passed to C++ compiler
ldflags = -L/home/doug/mobile_intelligence/baselines/development/lib,
#
#include malloc debugger
#  		/usr/lib/debug/malloc.o,
#
# include this to add the electric fence malloc debugger
#  		-static, -L/users/d/doug/lib, -lefence,
#
              CNL_LIBS,
	      -lhardware_drivers,
	      -lhardware,
	      -limage,
	      -lswarm,
	      -lg++,
#	      -lstdc++,
	      -lm

# *********** Real robot configuration flags ********************
#The list of real robots we know about
robots = ren, stimpy, george

# Any misc robot settings that will be dumped to the script file
MiscRobotSettings = "set show-trails on",
		    "set scale-robots on",
		    "set ROBOT-LENGTH 1.0" 


# Any list of strings attached to the robot name will be appended
# to the startup parameters for that robot.  The robot names are
# case sensitive.
ren    = "ignore_sensor23 = 1",
         "ignore_sensor1 = 1",
         "tty_num= 1"

stimpy = "ignore_sensor0 = 1",
         "ignore_sensor3 = 1",
         "ignore_sensor4 = 1",
         "ignore_sensor6 = 1",
         "tty_num= 0"

george = "robot_type = DRV1",
         "tty_num= 0"

