%define _unpackaged_files_terminate_build       0
%define _missing_doc_files_terminate_build      0

Name:		missionlabcarmen
Version:	1.0
Release:	1%{?dist}
Summary:	Missionlab and CARMEN integration

Group:		Applications/Others
License:	GPLv2+
Source0:	missionlabcarmen-%{version}.tar.gz

%if 0%{?suse_version}
Source1:	missionlabcarmen-rpmlintrc
%endif

BuildRoot:	%(mktemp -ud %{_tmppath}/%{name}-%{version}-%{release}-XXXXXX)

Requires:	xorg-x11-fonts-75dpi xorg-x11-fonts-100dpi binutils gcc-c++ gcc tcsh make zlib-devel glibc-devel bison flex ncurses-devel openssl-devel libv4l-devel gtk2-devel gnome-terminal libjpeg-devel libxml2-devel krb5-devel
BuildRequires:	xorg-x11-fonts-75dpi xorg-x11-fonts-100dpi binutils gcc-c++ gcc tcsh make zlib-devel glibc-devel bison flex ncurses-devel openssl-devel libv4l-devel gtk2-devel gnome-terminal libjpeg-devel libxml2-devel krb5-devel

%if 0%{?fedora}

  %if 0%{?fedora_version}
    %define real_fedora_ver %{fedora_version}
  %else
    %define real_fedora_ver %{fedora}
  %endif

  %if 0%{?real_fedora_ver} <= 23 
    Requires:	xorg-x11-fonts-misc motif-devel libstdc++-devel compat-flex libGLU-devel atlas
    BuildRequires:	xorg-x11-fonts-misc motif-devel libstdc++-devel compat-flex libGLU-devel atlas
  %else
    Requires:	xorg-x11-fonts-misc motif-devel libstdc++-devel compat-flex libGLU-devel atlas
    BuildRequires:	xorg-x11-fonts-misc motif-devel libstdc++-devel compat-flex libGLU-devel atlas flex-devel
  %endif

  #if ENABLE_OPENGL==1 -> libGLw-devel

%endif

%if 0%{?suse_version}

Requires:	xorg-x11-fonts-core openmotif-devel termcap
BuildRequires:	fdupes xorg-x11-fonts-core openmotif-devel termcap

# OpenSUSE Leap 42.1
%if 0%{?suse_version} == 1315	
Requires:	glu-devel libstdc++-devel inputproto-devel libXi-devel
BuildRequires:	glu-devel libstdc++-devel inputproto-devel libXi-devel
#if ENABLE_OPENGL==1 -> libGLw-devel 
%endif

%if 0%{?suse_version} >= 1320	
Requires:	glu-devel libstdc++-devel inputproto-devel libXi-devel
BuildRequires:	glu-devel libstdc++-devel inputproto-devel libXi-devel
#if ENABLE_OPENGL==1 -> libGLw-devel 
%endif

%if 0%{?suse_version} == 1310	
Requires:	glu-devel libstdc++-devel libXi-devel
BuildRequires:	glu-devel libstdc++-devel libXi-devel
#if ENABLE_OPENGL==1 -> libGLw-devel 
%endif

%if 0%{?suse_version} == 1230	
Requires:	glu-devel libstdc++47-devel
BuildRequires:	glu-devel libstdc++47-devel
#if ENABLE_OPENGL==1 -> libGLw-devel 
%endif

%if 0%{?suse_version} == 1220	
Requires:	Mesa-libGLU-devel libstdc++47-devel
BuildRequires:	Mesa-libGLU-devel libstdc++47-devel
#if ENABLE_OPENGL==1 -> libGLw-devel 
%endif

%if 0%{?suse_version} == 1210	
Requires:	Mesa-devel libstdc++43-devel
BuildRequires:	Mesa-devel libstdc++43-devel
#if ENABLE_OPENGL==1 -> MesaGLw-devel
%endif

%endif

%if 0%{?centos_version}
Requires:	xorg-x11-fonts-misc openmotif-devel libstdc++-devel libGLU-devel flex-devel
BuildRequires:	xorg-x11-fonts-misc openmotif-devel libstdc++-devel libGLU-devel flex-devel
#if ENABLE_OPENGL==1 -> libGLw-devel
%endif

%description
MissionLab takes high-level military-style plans and executes them with teams of real or simulated robotic vehicles. MissionLab supports execution of multiple robots both in simulation and actual robotics platforms, including device drivers for controlling iRobot's ATRV-Jr and Urban Robot, ActivMedia's AmigoBot and Pioneer AT, and Nomadics Technologies' Nomad 150 & 200. Each vehicle executes its portion of the mission using reactive control techniques developed at Georgia Tech. 

%prep
%setup -q -c

%build
rm -rf $RPM_BUILD_ROOT
# In the latest Fedora versions, there are certain issues compiling old code
# (without refactoring). C++23 standard changes how C handles certain cases,
# like void function pointers (used in map_graphics.c, example: line 455).
# https://fedoraproject.org/wiki/Changes/PortingToModernC
# Using the gnu89 standard avoids those issues, and it seems like the codebase
# doesn't require any modern C/C++ features.
export CFLAGS="%{optflags} -std=gnu89"
make

%install
export DESTDIR="$RPM_BUILD_ROOT"
make install

%if 0%{?suse_version}
%fdupes -s %{buildroot}
%endif

%clean
make clean

%files
/etc/*
/usr/bin/*
/usr/bin/.cfgeditrc
/usr/bin/.hserverrc
/usr/bin/.mission_expertrc
/usr/bin/.repair_planrc
/usr/data
/usr/demos
/usr/docs
/usr/include/*
/usr/lib/*
/usr/src

%exclude /usr/src/debug
%exclude /usr/lib/debug

%defattr(-,root,root,-)
%changelog

