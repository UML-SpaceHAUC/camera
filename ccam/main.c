// Copyright 2019 Sean Cummings
#include "ucam.h"

int main(int argc, char* argv[]) {
  int stream = open("/dev/ttyS0", O_RDWR | O_NOCTTY | O_NDELAY);
  // MAY NEED TO CHANGE PORT DEPENDING ON CONNECTOR!!
  if (stream < 0) {
    perror("Unable to open UART");
    exit(1);
  }

  struct termios options;
  tcgetattr(stream, &options);
  options.c_cflag = B115200 | CS8 | CLOCAL | CREAD;
  options.c_iflag = IGNPAR;
  options.c_oflag = 0;
  options.c_lflag = 0;
  tcflush(stream, TCIFLUSH);
  tcsetattr(stream, TCSANOW, &options);
  // These settings should always work for the UCAM-III
  char* str = "";
  int debug = 0;

  if (argv[2]) {
    debug = atoi(argv[2]);
  }

  if (!camera_Sync(stream, debug)) {
    if (debug)
      printf("failed to sync\n");
    exit(1);
  }

  if (!camera_Initialize(stream, debug)) {
    if (debug)
      printf("failed to init\n");
    exit(1);
  }

  if (!camera_Size(stream, debug)) {
    if (debug)
      printf("failed to size\n");
    exit(1);
  }

  if (!argv[1]) {
    printf("Please enter the filename in format:\nFILENAME.jpg\n");
    if (!scanf("%s", str)) {
      if (debug)
        printf("invalid filename.");
      exit(1);
    }
  } else {
    str = argv[1];
  }

  if (!camera_Jpg(stream, str, debug)) {
    if (debug)
      printf("failed to get\n");
    exit(1);
  }
}
