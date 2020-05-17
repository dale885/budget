#!/bin/bash

make clean


rm aclocal.m4
rm -fr autom4te.cache
rm compile
rm config.log
rm config.status
rm configure
rm depcomp
rm Makefile
rm Makefile.in
rm missing
rm install-sh

for dir in src tests Unity; do

	if [[ -f $dir/Makefile ]]; then
		rm $dir/Makefile
	fi

	if [[ -f $dir/Makefile.in ]]; then
		rm $dir/Makefile.in
	fi

	if [[ -d $dir/build ]]; then
		rm -r $dir/build
	fi
done


