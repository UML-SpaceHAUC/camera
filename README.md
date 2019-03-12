# UCAM-III camera

This repository contains Python and C code which allows users to
take pictures with the UCAM-III camera. This code is part of the
UMASS Lowell SpaceHAUC project, and is free to be used by anybody.
Most instructions are found in the code files themselves.

## Directory Structure
### pycam: Python code for the camera.

Currently, the jpeg.py file is set up to take a 640x480 JPEG and save it to a file
called test.jpg.

The test.py file takes a RAW photo and saves it as test.RAW.

### ccam: C code for the camera.
The Makefile makes a program called ucam from ucam.c and ucam.h which
takes a 640x480 JPEG and saves it to a file named based off user input.

All settings for the camera are currently hardcoded. Refer to comments/datasheet/User guide
for uses outside the current capabilities.

## To run
In C (ccam):
make
./ucam filename.jpg int

Where filename is the name of the file you wish to create, and int is either a
1 or a 0, 1 for debug info, 0 for no debug info.

In python (pycam):
./jpeg.py

Creates a JPEG file called test.jpg.

./test.py

Creates a RAW file called test.RAW.

## References:
https://www.4dsystems.com.au/productpages/uCAM-III/downloads/uCAM-III_datasheet_R_1_0.pdf
