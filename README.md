# STACS Image

This program reads a text file containing locations of image files (e.g. `.png`) and sends the image signal to STACS through YARP.

There are also some testing programs (adapted from the YARP examples) to send and receive image data.

Building using CMake:

	mkdir build
	cd build
	ccmake ../
	make
