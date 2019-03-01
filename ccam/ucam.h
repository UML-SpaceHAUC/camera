// Copyright Sean Cummings 2019
#ifndef CCAM_UCAM_H_
#define CCAM_UCAM_H_

#include <fcntl.h>
#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <termios.h>
#include <unistd.h>

int camera_Sync(int stream, int debug);
int camera_Initialize(int stream, int debug);
int camera_Size(int stream, int debug);
int camera_Jpg(int stream, char* str, int debug);

int camera_Sync(int stream, int debug) {
  char _SYNC_COMMAND[6] = {0xAA, 0x0D, 0x00, 0x00, 0x00, 0x00};
  char _SYNC_ACK_REPLY[6] = {0xAA, 0x0E, 0x0D, 0x00, 0x00, 0x00};
  char _SYNC_ACK_REPLY_EXT[6] = {0xAA, 0x0D, 0x00, 0x00, 0x00, 0x00};
  char _CAMERA_INIT[] = {0xAA, 0x01, 0x00, 0x07, 0x03, 0x07};
  char _CAMERA_INIT_ACK[] = {0xAA, 0x0E, 0x01, 0x00, 0x00, 0x00};

  char inbuff[6];

  int count = 0;
  unsigned int time = 5000;

  for (int i = 0; i < 60; i++) {
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
          sleep(2);
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
  sleep(1);
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
  sleep(1);
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
  char _GET[] = {0xAA, 0x04, 0x05, 0x00, 0x00, 0x00};
  char _GET_ACK[] = {0x0AA, 0x0E, 0x04, 0x00, 0x00, 0x00};
  char _DATA[] = {0xAA, 0x0A, 0x05, 0x00, 0x00, 0x00};
  char _DACK[] = {0xAA, 0x0E, 0x00, 0x00, 0x00, 0x00};
  char received[512];
  char inbuff[6];
  FILE *img;
  int count;
  img = fopen(str, "wb");

  count = write(stream, _GET, 6);
  if (count < 0) {
    if (debug)
      printf("###### WRITE STREAM ERROR ######\nEXITING\n");
    exit(1);
  }
  sleep(1);
  count = read(stream, (void*) inbuff, 6);
  if (count < 0) {
    if (debug)
      printf("###### READ ERROR ######\nEXITING\n");
    exit(1);
  }
  if (debug) {
    for (int j = 0; j < 6; ++j) {
      printf("inbuff[%d] = 0x%X == SYNCREP = 0x%X\n",
      j, inbuff[j], _GET_ACK[j]);
    }
  }
  if (inbuff[0] == _GET_ACK[0] && inbuff[1] == _GET_ACK[1]
  && inbuff[2] == _GET_ACK[2]) {
    sleep(2);
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
      sleep(5);
      int size = inbuff[5];
      int temp = inbuff[4];
      temp = temp * 256;
      size += temp;
      temp = inbuff[3];
      temp = temp * 65536;
      size += temp;
      if (debug)
        printf("size = %d\n", size);
      int nump = size/(512 - 6);

      count = write(stream, _DACK, 6);
      if (count < 0) {
        if (debug)
          printf("###### WRITE FILE ERROR ######\nEXITING\n");
        exit(1);
      }
      sleep(1);
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
      int pcknum = 0;
      while (pcknum < nump) {
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
      _DACK[4] = received[0];
      _DACK[5] = received[1];
      write(stream, _DACK, 6);
      usleep(500000);
      count = read(stream, (void*) received, 512);
      if (count < 0) {
        if (debug)
          printf("###### READ ERROR ######\nEXITING\n");
        exit(1);
      }
  // pcknum = received[0];
      temp = received[3];
      temp *= 256;
      pcknum += temp;
      printf("%d, %d\n", pcknum, nump);
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


#endif  // CCAM_UCAM_H_
