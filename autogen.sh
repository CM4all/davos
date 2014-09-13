#!/bin/sh

set -e

rm -rf config.cache build
mkdir build
aclocal
automake --add-missing --foreign
autoconf
./configure \
	CC=clang CXX=clang++ \
	CFLAGS="-O0 -ggdb" CXXFLAGS="-O0 -ggdb" \
        --prefix=/usr/local/stow/cm4all-filters \
	--enable-debug \
	--enable-silent-rules \
	"$@"
