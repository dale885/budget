#!/bin/bash

make clean

clean() {
	for entry in $1/*; do

		if [ -d $entry/build ]; then
			clean $entry
		fi

		if [[ -f $entry/Makefile ]]; then
			rm -v $entry/Makefile
		fi

		if [[ -f $entry/Makefile.in ]]; then
			rm -v $entry/Makefile.in
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

for dir in src tests; do
	clean $dir
done

if [[ -d Unity/build ]]; then
	rm -rv Unity/build
fi

