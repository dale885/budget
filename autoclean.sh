#!/bin/bash

make clean

clean() {
	for entry in $1/*; do
		if [ -d $entry ]; then
			clean $entry
		fi

		if [[ -f $entry/Makefile ]]; then
			rm $entry/Makefile
		fi

		if [[ -f $entry/Makefile.in ]]; then
			rm $entry/Makefile.in
		fi

		if [[ -d $entry/build ]]; then
			rm -r $entry/build
		fi
	done
}

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
	clean $dir
done

