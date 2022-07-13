#!/usr/bin/env bash

# ARM_MIRROR=https://github.com/DLTcollab/toolchain-arm/raw/main
# https://github.com/DLTcollab/toolchain-arm/raw/main

BASE_URL=https://github.com/d0iasm/rvemu/raw/main/bin/xv6

KERNEL_CHECKSUM=7dd1fe7f470389ac991d1615aaf8ac7fc6731330
ROOTFS_CHECKSUM=27fd366b4383f1176f44095fa270d23e9b3f4deb

echo "Downloading..."

wget -q -c $BASE_URL/kernel.bin || exit 1
echo "${KERNEL_CHECKSUM}  kernel.bin" | shasum -c || exit 1

wget -q -c $BASE_URL/fs.img || exit 1
echo "${ROOTFS_CHECKSUM}  fs.img" | shasum -c || exit 1
