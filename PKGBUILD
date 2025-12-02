# Maintainer: pblop <aur at pabl dot eu>
pkgname="missionlabcarmen-git"
pkgver=r9.a3328c2
pkgrel=1
pkgdesc="MissionLab and Carmen integration. Original source code from https://build.opensuse.org/package/show/home:fjaviersr/missionlabcarmen"
arch=('x86_64' 'arm64')
url="https://github.com/pblop/missionlab-carmen"
license=('GPLv2+')
depends=(
  "base-devel" "tcsh" "gnome-terminal" "xorg-fonts-75dpi" "xorg-fonts-100dpi"
  "zlib" "glibc" "ncurses" "openssl" "v4l-utils" "gtk2" "libjpeg-turbo"
  "libxml2" "krb5" "xorg-fonts-misc" "openmotif" "libtirpc"
)
makedepends=("git" "glu" "flex" "make" "gcc")
provides=(
  "camera_fakecam" "camera_view" "carmen-config" "cbrserver" "cdl" "central"
  "cfgedit" "cnl" "create_hmap" "EventLogParser" "gps-nmea" "hmap" "hserver"
  "iptserver" "l3d2log" "laser" "laserfit" "laserview" "localize"
  "localize_initialize" "log_carmen" "log_carmen_comment" "logtool_carmen" "map"
  "maptool" "mlab" "mlab-2.0-robot" "model_learner" "navigator" "navigatorgui"
  "orc4" "orc5" "pantilt" "pantilt-test" "param_daemon" "param_edit"
  "param_tool" "path_planner" "pioneer" "play_carmen" "play_carmen_control"
  "proccontrol" "proccontrol_setgroup" "proccontrol_setmodule"
  "proccontrol_viewoutput" "rflex" "robot" "robotgui" "rta" "scout"
  "sick_laser_init_500k" "simulator" "simulator_connect_multiple"
  "test_gui_config" "tmr_robot-1998" "vasco" "vasco-tiny" "wingman_control"
)
conflicts=()
source=("missionlab-carmen-$pkgver::git+https://github.com/pblop/missionlab-carmen.git")
sha256sums=('SKIP')

pkgver() {
  cd "$srcdir/missionlab-carmen-$pkgver" || exit
  ( set -o pipefail
    git describe --long --abbrev=7 2>/dev/null | sed 's/\([^-]*-g\)/r\1/;s/-/./g' ||
    printf "r%s.%s" "$(git rev-list --count HEAD)" "$(git rev-parse --short=7 HEAD)"
  )
}

prepare() {
  cd "$srcdir/missionlab-carmen-$pkgver"
  # With -lfl, the linker complains about "yywrap" not being defined.
  find . \( -iname "Makefile" -o -name "*.include" \) -exec sed -i 's/ -lfl//g' {} +
  # Not a standalone library anymore
  find . \( -iname "Makefile" -o -name "*.include" \) -exec sed -i 's/ -ltermcap//g' {} +
  # find . \( -iname "Makefile" -o -name "*.include" \) -exec sed -i 's/-ansi//g' {} +
  # The bundled rpc_main.c hardcodes "/lib/cpp". We patch it to "/usr/bin/cpp".
  # This fixes the corrupt generation of ipc_xdr.cc
  find . -name "rpc_main.c" -exec sed -i 's|/lib/cpp|/usr/bin/cpp|g' {} +

  # Termio is no longer used in modern Linux systems. Change to termios.
  find . \( -name "*.c" -o -name "*.h" \) -exec sed -i 's/<termio.h>/<termios.h>/g' {} +
  find . \( -name "*.c" -o -name "*.h" \) -exec sed -i 's/struct termio\b/struct termios/g' {} +
  find . \( -name "*.c" -o -name "*.h" \) -exec sed -i 's/TCGETA/TCGETS/g' {} +
  find . \( -name "*.c" -o -name "*.h" \) -exec sed -i 's/TCSETA/TCSETS/g' {} +
}

build() {
  cd "$srcdir/missionlab-carmen-$pkgver" || exit
  mkdir -p wrappers

  cat > wrappers/gcc <<EOF
#!/bin/bash
/usr/bin/gcc $@" \\
    -fdiagnostics-color=always \\
    -Dlinux \\
    -I/usr/include/tirpc \\
    -std=gnu99 -Wno-error=incompatible-pointer-types -Wno-implicit-function-declaration -include stdbool.h \\
    -std=gnu++98 -fpermissive \\
    2> >(grep -v "command-line option.*is valid for.*but not for" >&2)
EOF
  chmod +x wrappers/gcc

  cat > wrappers/g++ <<EOF
#!/bin/bash
exec /usr/bin/g++ $@" \\
    -fdiagnostics-color=always \\
    -Dlinux \\
    -I/usr/include/tirpc \\
    -std=gnu++98 -fpermissive
EOF
  chmod +x wrappers/g++
  ln -sf gcc wrappers/cc
  ln -sf g++ wrappers/c++
  ln -sf g++ wrappers/cpp

  PATH="$PWD/wrappers:$PATH" make
}

package() {
  cd "$srcdir/missionlab-carmen-$pkgver" || exit
  mkdir -p "$pkgdir/usr/bin"
  make INSTALL_DIR="$pkgdir/usr/bin" install
}