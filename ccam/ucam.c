// Copyright 2019 Sean Cummings
#include "ucam.h"

/* This code was produced as part of the UMass Lowell
SPACEHAUC project. It is open for use/modificaton by anyone
for any purpose.

This file along with it's header (ucam.h) are set up
to provide communications with and commands for the
UCAM-III (MicroCAM 3) by 4d systems. All commands are
written to work as detailed in the datasheet, found at
https://www.4dsystems.com.au/productpages/uCAM-III/downloads/uCAM-III_datasheet_R_1_0.pdf

Please see the USERGUIDE.md for changes you can make to the code
to enable other features of the camera.
*/

int camera_Sync(int stream, int debug) {
  /* This function synchronizes the camera with the computer/device.
  There is not much variation in this function, although the camera
  can reply with various NACK responses, which can be found in the datasheet.
  */
  char _SYNC_COMMAND[6] = {0xAA, 0x0D, 0x00, 0x00, 0x00, 0x00};
  char _SYNC_ACK_REPLY[6] = {0xAA, 0x0E, 0x0D, 0x00, 0x00, 0x00};
  char _SYNC_ACK_REPLY_EXT[6] = {0xAA, 0x0D, 0x00, 0x00, 0x00, 0x00};

  char inbuff[6];

  int count = 0;
  unsigned int time = 5000;

  for (int i = 0; i < 60; i++) {
      // Datasheet claims up to 60 requests necessary
      count = write(stream, _SYNC_COMMAND, 6);
      if (count < 0) {
        if (debug)
          perror("can't count");
        exit(1);
      }
    count = read(stream, (void*) inbuff, 6);
    if (count < 0) {
      if (debug)
        printf("###### READ ERROR ######\nEXITING\n");
      exit(1);
    }

  // DEBUgg: Compares what you receive with what you expect
    if (debug) {
      for (int j = 0; j < 6; ++j) {
        printf("inbuff[%d] = 0x%X == SYNCREP = 0x%X\n", j, inbuff[j],
        _SYNC_ACK_REPLY[j]);
      }
    }
  // ENDDEBUG

    if (inbuff[0] == _SYNC_ACK_REPLY[0] && inbuff[1] ==
      _SYNC_ACK_REPLY[1] && inbuff[2] == _SYNC_ACK_REPLY[2]) {
      if (debug) {
        printf("We got ack!! \n Checking for SYNC...\n");
      }
      count = read(stream, (void*) inbuff, 6);
      if (count < 0) {
        if (debug)
          printf("###### READ ERROR ######\nEXITING\n");
        exit(1);
      }
      if (inbuff[0] == _SYNC_ACK_REPLY_EXT[0] && inbuff[1] ==
      _SYNC_ACK_REPLY_EXT[1]) {
        if (debug) {
          printf("We got SYNC!!\n Sending ack...\n");
        }
        count = write(stream, _SYNC_ACK_REPLY, 6);
        if (count < 0) {
          if (debug)
            printf("###### WRITE ERROR ######\nRETRYING\n");
        } else {
          if (debug) {
            printf("Successfull SYNC!\n");
          }
          i = 60;
          sleep(2); // As specified in datasheet
          return 1;
        }
      }
    }
  time += 1000;
  usleep(time);
  }
  return 0;
}

int camera_Initialize(int stream, int debug) {
  /* This function initializes the camera to take a 420 x 680
  JPEG. In order to take a RAW image or change the resolution,
  please refer to the USERGUIDE.md .
  */
  char _CAMERA_INIT[] = {0xAA, 0x01, 0x00, 0x07, 0x03, 0x07};
  char _CAMERA_INIT_ACK[] = {0xAA, 0x0E, 0x01, 0x00, 0x00, 0x00};
  int count = 0;
  char inbuff[6];

  count = write(stream, _CAMERA_INIT, 6);
  if (count < 0) {
    if (debug)
      printf("###### WRITE ERROR ######\nEXITING\n");
    exit(1);
  }
  usleep(50000);
  count = read(stream, (void*) inbuff, 6);
  if (debug) {
    for (int j = 0; j < 6; ++j) {
      printf("inbuff[%d] = 0x%X == SYNCREP = 0x%X\n", j, inbuff[j],
      _CAMERA_INIT_ACK[j]);
    }
  }
  if (count < 0) {
    if (debug)
      printf("###### READ ERROR ######\nEXITING\n");
    exit(1);
  }
  if (inbuff[0] == _CAMERA_INIT_ACK[0] && inbuff[1] ==
  _CAMERA_INIT_ACK[1] && inbuff[2] == _CAMERA_INIT_ACK[2]) {
    if (debug)
      printf("Camera Initialized\n");
    return 1;
  }
  return 0;
}

int camera_Size(int stream, int debug) {
  /* This function sets the package size of JPEG images to
  512 bytes. For other configurations, refer to the
  USERGUIDE.md and the datasheet.
  */
  char _SIZE[] = {0xAA, 0x06, 0x08, 0x00, 0x02, 0x00};
  char _SIZE_ACK[] = {0xAA, 0x0E, 0x06, 0x00, 0x00, 0x00};

  int count = 0;
  char inbuff[6];

  count = write(stream, _SIZE, 6);
  if (count < 0) {
    if (debug)
      printf("###### WRITE ERROR ######\nEXITING\n");
    exit(1);
  }
  usleep(50000); // Hardware restriction
  count = read(stream, (void*) inbuff, 6);
  if (debug) {
    for (int j = 0; j < 6; ++j) {
      printf("inbuff[%d] = 0x%X == SYNCREP = 0x%X\n",
      j, inbuff[j], _SIZE_ACK[j]);
    }
  }
  if (count < 0) {
    if (debug)
      printf("###### READ ERROR ######\nEXITING\n");
    exit(1);
  }
  if (inbuff[0] == _SIZE_ACK[0] && inbuff[1] == _SIZE_ACK[1]
  && inbuff[2] == _SIZE_ACK[2]) {
    if (debug)
      printf("Camera Sized\n");
    return 1;
  }
  return 0;
}


int camera_Jpg(int stream, char* str, int debug) {
  /* This function retrieves the JPEG from the camera. It is currently
  setup for siz 512 byte packages, and may not work for smaller sizes.
  Refer to USERGUIDE.md and the datasheet for further details.
  */
  char _GET[] = {0xAA, 0x04, 0x05, 0x00, 0x00, 0x00};
  char _GET_ACK[] = {0x0AA, 0x0E, 0x04, 0x00, 0x00, 0x00};
  char _DATA[] = {0xAA, 0x0A, 0x05, 0x00, 0x00, 0x00};
  char _DACK[] = {0xAA, 0x0E, 0x00, 0x00, 0x00, 0x00};
  char received[512];
  char inbuff[6];
  FILE *img;
  int count;
  int pcknum = 1;
  int size = 0;
  int temp = 0;
  img = fopen(str, "wb");

  count = write(stream, _GET, 6);
  if (count < 0) {
    if (debug)
      printf("###### WRITE STREAM ERROR ######\nEXITING\n");
    exit(1);
  }
  usleep(80000);
  count = read(stream, (void*) inbuff, 6);
  if (count < 0) {
    if (debug)
      printf("###### READ ERROR ######\nEXITING\n");
    exit(1);
  }
  if (debug) {
    for (int j = 0; j < 6; ++j) {
      printf("inbuff[%d] = 0x%X == GET_ACK = 0x%X\n",
      j, inbuff[j], _GET_ACK[j]);
    }
  }
  if (inbuff[0] == _GET_ACK[0] && inbuff[1] == _GET_ACK[1]
  && inbuff[2] == _GET_ACK[2]) {
    usleep(50000);
    count = read(stream, (void*) inbuff, 6);
    if (debug) {
      for (int j = 0; j < 6; ++j) {
        printf("inbuff[%d] = 0x%X == SYNCREP = 0x%X\n", j, inbuff[j], _DATA[j]);
      }
    }
    if (count < 0) {
      if (debug)
        printf("###### READ ERROR ######\nEXITING\n");
      exit(1);
    }
    if (inbuff[0] == _DATA[0] && inbuff[1] == _DATA[1]
    && inbuff[2] == _DATA[2]) {
      usleep(50000);
      size += inbuff[5];
      temp = inbuff[4];
      temp = temp * 256;
      size += temp;
      temp = inbuff[3];
      temp = temp * 65536;
      size += temp;
      if (debug)
        printf("size = %d\n", size); //FILESIZE in bytes
      count = write(stream, _DACK, 6);
      if (count < 0) {
        if (debug)
          printf("###### WRITE FILE ERROR ######\nEXITING\n");
        exit(1);
      }
      usleep(50000);
      count = read(stream, (void*) received, 512);
      if (count < 0) {
        if (debug)
          printf("###### READ ERROR ######\nEXITING\n");
        exit(1);
      }
      temp = received[3];
      size = received[4];
      temp *= 256;
      size += temp;
      _DACK[4] = received[0];
      _DACK[5] = received[1];
      while (pcknum != 0) {
        char tempr[506];
        int j = 0;
        for (int k = 4; k < 510; k++) {
          tempr[j] = received[k];
          j++;
        }
      count = fwrite(&tempr, 1, sizeof(tempr), img);
      if ((count <= 0)) {
        if (debug)
          printf("###### WRITE FILE ERROR ######\nEXITING\n");
        exit(1);
      }
      if (write(stream, _DACK, 6) < 0) {
        if (debug)
          printf("Failed to Write.");
        exit(1);
      }
      usleep(50000);
      count = read(stream, (void*) received, 512);
      _DACK[4] = received[0];
      _DACK[5] = received[1];
      if (debug)
        printf("pckgsize = %d\n", count);
      pcknum = count;
      if (count < 0) {
        if (debug)
          printf("###### READ ERROR ######\nEXITING\n");
        exit(1);
      }
    }
  if (write(stream, _DACK, 6) < 0) {
    if (debug)
      printf("Failed to Write.");
    exit(1);
  }
  fclose(img);
  return 1;
  }
}
fclose(img);
if (debug)
  printf("failed.\n");
return 0;
}
