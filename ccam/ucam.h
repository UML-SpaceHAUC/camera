// Copyright Sean Cummings 2019
#ifndef CCAM_UCAM_H_
#define CCAM_UCAM_H_

#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <termios.h>
#include <unistd.h>

int camera_Sync(int stream, int debug);
int camera_Initialize(int stream, int debug);
int camera_Size(int stream, int debug);
int camera_Jpg(int stream, char* str, int debug);

#endif  // CCAM_UCAM_H_
