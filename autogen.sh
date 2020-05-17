#!/bin/bash -e

UNITY_DIR=Unity
BUILD_DIR=build

if [ ! -d $UNITY_DIR/$BUILD_DIR ]; then
	mkdir $UNITY_DIR/$BUILD_DIR
fi
	cd $UNITY_DIR/$BUILD_DIR

	cmake -G "Unix Makefiles" ..
	make
	cd ../../

autoreconf -i

