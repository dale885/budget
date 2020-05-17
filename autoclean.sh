#!/bin/bash

make clean


rm aclocal.m4
rm -fr autom4te.cache
rm compile
rm config.log
rm config.status
rm configure
rm depcomp
rm install.sh
rm Makefile
rm Makefile.in
rm missing

for dir in src,tests,Unity; do

	rm $dir/Makefile
	rm $dir/Makefile.in
done

rm -fr Unity/build

