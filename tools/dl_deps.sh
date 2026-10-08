#!/bin/sh
# Download the prebuilt tools into bin/ and check them against tools/deps.sha256

set -e

TOP="$(dirname "$(dirname "$(readlink -f -- "$0")")")"
BIN_DIR="$TOP/bin"

mkdir -p "$BIN_DIR"
cd "$BIN_DIR"

fetch() {
	# GitHub's release downloads fail now and then (wget exit 8): retry a few times
	for try in 1 2 3 4 5; do
		rm -f "$1"
		wget -q -O "$1" "$2" && break
		[ "$try" = 5 ] && exit 1
		sleep $((try * 5))
	done
	grep " $1\$" "$TOP/tools/deps.sha256" | sha256sum -c -
}

fetch gcc-2.95.2-psx.tar.gz https://github.com/decompals/old-gcc/releases/download/0.17/gcc-2.95.2-psx.tar.gz
rm -rf gcc-2.95.2-psx
mkdir gcc-2.95.2-psx
tar -xzf gcc-2.95.2-psx.tar.gz -C gcc-2.95.2-psx
rm gcc-2.95.2-psx.tar.gz

fetch objdiff-cli-linux-x86_64 https://github.com/encounter/objdiff/releases/download/v3.8.1/objdiff-cli-linux-x86_64
chmod a+x objdiff-cli-linux-x86_64
