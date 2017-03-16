export TARGET_ARCH=i386
export SYSROOT="$(pwd)/sysroot"
export PROJECTS="libc kernel"
export MAKE=${MAKE:-make}
export CC=i686-elf-gcc
export AR=i686-elf-ar
