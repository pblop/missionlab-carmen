all:
	export DESTDIR=/usr ; \
	export ENABLE_OPENGL=0 ; \
	export ENABLE_MOBILITY=0 ; \
	export ENABLE_OPENCV=1 ; \
	export ENABLE_GAZEBO=0 ; \
	export USE_IPC=1 ; \
	export BUILD_HOME=`pwd` && cd src && ./configure && make
	-mkdir include/carmen
	-ln -s ../ipc.h include/carmen/ipc.h
	export BUILD_HOME=`pwd` && cd src && make -f MakefileCarmen 

install:	
	cd src && make install

clean:
	cd src && make clean

