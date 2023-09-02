#!/bin/bash
# Script outline to install and build kernel.
# Author: Siddhant Jajoo.

set -e
set -u

OUTDIR=/tmp/aeld
KERNEL_REPO=git://git.kernel.org/pub/scm/linux/kernel/git/stable/linux-stable.git
KERNEL_VERSION=v5.1.10
BUSYBOX_VERSION=1_33_1
FINDER_APP_DIR=$(realpath $(dirname $0))
ARCH=arm64
CROSS_COMPILE=aarch64-none-linux-gnu-
WRITER_PATH=$(dirname $(realpath -e $BASH_SOURCE))

if [ $# -lt 1 ]
then
	echo "Using default directory ${OUTDIR} for output"
else
	OUTDIR=$1
	echo "Using passed directory ${OUTDIR} for output"
fi

mkdir -p ${OUTDIR}

cd "$OUTDIR"
if [ ! -d "${OUTDIR}/linux-stable" ]; then
    #Clone only if the repository does not exist.
	echo "CLONING GIT LINUX STABLE VERSION ${KERNEL_VERSION} IN ${OUTDIR}"
	git clone ${KERNEL_REPO} --depth 1 --single-branch --branch ${KERNEL_VERSION}
fi
if [ ! -e ${OUTDIR}/linux-stable/arch/${ARCH}/boot/Image ]; then
    cd linux-stable
    echo "Checking out version ${KERNEL_VERSION}"
    git checkout ${KERNEL_VERSION}

    # TODO: Add your kernel build steps here
    for action in mrproper defconfig all modules dtbs;
    do
        test -f ${OUTDIR}/linux-stable/.config && [ ${action} = "mrproper" ] && continue;
        [ ${action} = "all" ] && jobs="-j4" || jobs=
        make ${jobs} ARCH=${ARCH} CROSS_COMPILE=${CROSS_COMPILE} ${action}
    done
    # This line is to deep clean the kernel build tree (remove the .config file)
    
    # make ARCH=${ARCH} CROSS_COMPILE=${CROSS_COMPILE} mrproper

    # This line is configure for "virt" arm dev borad (simulate in QEMU)
    # make ARCH=$ARCH CROSS_COMPILE=${CROSS_COMPILE} defconfig
    # This line is to build the kernel image for booting with QEMU

    # make -j4 ARCH=${ARCH} CROSS_COMPILE=${CROSS_COMPILE} all 

    # This line is to build any kernel modules
    # make ARCH=$ARCH CROSS_COMPILE=${CROSS_COMPILE} modules
    # This line is to build device tree
    # make ARCH=$ARCH CROSS_COMPILE=${CROSS_COMPILE} dtbs
fi

echo "Adding the Image in outdir"
#copy the image into outdir
cp -v ${OUTDIR}/linux-stable/arch/${ARCH}/boot/Image ${OUTDIR}

echo "Creating the staging directory for the root filesystem"
cd "$OUTDIR"
if [ -d "${OUTDIR}/rootfs" ]
then
	echo "Deleting rootfs directory at ${OUTDIR}/rootfs and starting over"
    sudo rm  -rf ${OUTDIR}/rootfs
fi

# TODO: Create necessary base directories
for dir in bin boot dev etc home lib usr usr/bin usr/sbin sbin dev proc sys opt;
do
    mkdir -vp ${OUTDIR}/rootfs/${dir}
done

cd "$OUTDIR"
if [ ! -d "${OUTDIR}/busybox" ]
then
    git clone git://busybox.net/busybox.git
    cd busybox
    git checkout ${BUSYBOX_VERSION}
    # TODO:  Configure busybox
    # Delete all files in the current directory (or created by this makefile)
    # that are created by configuring or building the program
    make distclean 
    # if option is mentioned in defconfig, build system puts that option into .config with value chosen in defconfig
    # if option isn't mentioned in defconfig, build system puts that option into .config using its default value, specified in corresponding Kconfig
    make ARCH=${ARCH} CROSS_COMPILE=${CROSS_COMPILE} defconfig
else
    cd busybox
fi

# TODO: Make and install busybox
sudo make ARCH=${ARCH} CROSS_COMPILE=${CROSS_COMPILE} CONFIG_PREFIX="${OUTDIR}/rootfs" install

echo "Library dependencies"
# find content from the result of readelf
program_interpreter=$(${CROSS_COMPILE}readelf -l ${OUTDIR}/rootfs/bin/busybox | \
 grep -Eo "ld-[[:alnum:]]*-[[:alnum:]]*.so.[0-9]*")
shared=$(${CROSS_COMPILE}readelf -l ${OUTDIR}/rootfs/bin/busybox | \
 grep -Eow "lib[[:alnum:]]*\.so\.[[:digit:]]*")


# ${CROSS_COMPILE}readelf -a bin/busybox | grep "Shared library"

# TODO: Add library dependencies to rootfs
for dep in ${program_interpreter} ${shared};
do 
    find ${WRITER_PATH}/ -name "${dep}" -exec \ 
    cp -v {} ${OUTDIR}/rootfs/lib;
done
if [ $? -eq 0 ]; then
	echo "Failed in adding library dependencies to rootfs"
fi

# TODO: Make device nodes
sudo mkdir ${OUTDIR}/rootfs/dev/null c 1 3
sudo mkdir ${OUTDIR}/rootfs/dev/console c 5 1

# TODO: Clean and build the writer utility
cd WRITER_PATH
make clean
make CROSS_COMPILE=${CROSS_COMPILE} writer

# TODO: Copy the finder related scripts and executables to the /home directory
# on the target rootfs
for file in writer finder.sh finder-test.sh autorun-qemu-app.sh conf;
do
    if [ -L $(file) ]
    then
        cp -vr $(realpath -e ${file}) $(OUTDIR)/rootfs/home
    else
        cp -vr ${WRITER_PATH}/${file} $(OUTDIR)/rootfs/home
    if
done

# TODO: Chown the root directory
sudo chown root:root -R ${OUTDIR}/rootfs/*
# TODO: Create initramfs.cpio.gz
( cd ${OUTDIR}/rootfs; find . | cpio --create --format-newc | gzip) > \
 ${OUTDIR}/initramfs.cpio.gz