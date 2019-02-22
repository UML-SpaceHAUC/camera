#include <stdio.h>
#include <unistd.h>
#include <stdint.h>
#include <fcntl.h>
#include <termios.h>
#include <stdlib.h>
#include <string.h>

//char _SYNC_FINAL_COMMAND[6] = {0xAA, 0x0E, 0x00, 0x00, 0xF5, 0x00};
//char _INITIAL_COMMAND[6] = {0xAA, 0x01, 0x00, 0x07, 0x03, 0x07};
//char _GENERIC_ACK_REPLY[6] = {0xAA, 0x0E, 0x00, 0x00, 0x00, 0x00};
//char _PACK_SIZE[6] = {0xAA, 0x06, 0x08, 0x00, 0x02, 0x00};
//char _GET_PICTURE[6] = {0xAA, 0x04, 0x01, 0x00, 0x00, 0x00};
//char _SNAPSHOT[6] = {0xAA, 0x05, 0x00, 0x00, 0x00, 0x00};

int camera_Sync(int stream);
int camera_Initialize(int stream);
int camera_Size(int stream);
int camera_Jpg(int stream, char* str);

int main(int argc, char* argv[]) {
	int stream = open("/dev/ttyS0", O_RDWR | O_NOCTTY | O_NDELAY);
	if (stream < 0 ) {
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

	char str;

	if(!camera_Sync(stream)) {
		printf("failed to sync\n");
		exit(1);
	}
	printf("synced\n");
	sleep(2);
	if(!camera_Initialize(stream)) {
		printf("failed to init\n");
		exit(1);
	}
	if(!camera_Size(stream)) {
		printf("failed to size\n");
		exit(1);
	}
	printf("Please enter the filename in format:\nFILENAME.jpg\n");
	scanf("%s", &str);
	if(!camera_Jpg(stream, &str)) {
		printf("failed to get\n");
		exit(1);
	}
}

int camera_Sync(int stream) {

	char _SYNC_COMMAND[6] = {0xAA, 0x0D, 0x00, 0x00, 0x00, 0x00};
	char _SYNC_ACK_REPLY[6] = {0xAA, 0x0E, 0x0D, 0x00, 0x00, 0x00};
	char _SYNC_ACK_REPLY_EXT[6] = {0xAA, 0x0D, 0x00, 0x00, 0x00, 0x00};
	char _CAMERA_INIT[] = {0xAA, 0x01, 0x00, 0x07, 0x03, 0x07};
	char _CAMERA_INIT_ACK[] = {0xAA, 0x0E, 0x01, 0x00, 0x00, 0x00};

	char inbuff[6];

	int i = 0;
	int count = 0;
	int j = 0;
	int k = 0;
	int succ;
	unsigned int time = 5000;
	for (i = 0; i < 60; i++) {
			count = write(stream, _SYNC_COMMAND, 6);
			if (count < 0)
				perror("can't count");

		count = read(stream, (void*) inbuff, 6);

		if (count < 0) {
			printf("###### READ ERROR ######\nEXITING\n");
			exit(1);
		}

//FOR DEBUGGING
//		for (j = 0; j < 6; ++j) {
//			printf("inbuff[%d] = 0x%X == SYNCREP = 0x%X\n", j, inbuff[j], _SYNC_ACK_REPLY[j]);
//		}

		if (inbuff[0] == _SYNC_ACK_REPLY[0] && inbuff[1] == _SYNC_ACK_REPLY[1] && inbuff[2] == _SYNC_ACK_REPLY[2]) {
					printf("We got ack!! \n Checking for SYNC...\n");
					count = read(stream, (void*) inbuff, 6);
					if (count < 0) {
						printf("###### READ ERROR ######\nEXITING\n");
						exit(1);
					}
					if (inbuff[0] == _SYNC_ACK_REPLY_EXT[0] && inbuff[1] == _SYNC_ACK_REPLY_EXT[1]) {
						printf("We got SYNC!!\n Sending ack...\n");
						count = write(stream, _SYNC_ACK_REPLY, 6);
						if (count < 0) {
							printf("###### WRITE ERROR ######\nRETRYING\n");
						} else {
							printf("Successfull SYNC!\n");
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

int camera_Initialize(int stream) {
	char _CAMERA_INIT[] = {0xAA, 0x01, 0x00, 0x07, 0x03, 0x07};
	char _CAMERA_INIT_ACK[] = {0xAA, 0x0E, 0x01, 0x00, 0x00, 0x00};
	int count = 0;
	char inbuff[6];

	count = write(stream, _CAMERA_INIT, 6);
	if (count < 0) {
		printf("###### WRITE ERROR ######\nEXITING\n");
		exit(1);
	}
	for (int i = 0; i < 60; i++) {
		count = read(stream, (void*) inbuff, 6);
		//FOR DEBUGGING
				for (int j = 0; j < 6; ++j) {
					printf("inbuff[%d] = 0x%X == SYNCREP = 0x%X\n", j, inbuff[j], _CAMERA_INIT_ACK[j]);
				}
		if (count < 0) {
			printf("###### READ ERROR ######\nEXITING\n");
			exit(1);
		}
		//printf(".");
		if (inbuff[0] == _CAMERA_INIT_ACK[0] && inbuff[1] == _CAMERA_INIT_ACK[1] && inbuff[2] == _CAMERA_INIT_ACK[2]) {
			printf("Camera Initialized\n");
			return 1;
		}
	}
	return 0;
}

int camera_Size(int stream) {
	char _SIZE[] = {0xAA, 0x06, 0x08, 0x00, 0x02, 0x00};
	char _SIZE_ACK[] = {0xAA, 0x0E, 0x06, 0x00, 0x00, 0x00};

	int count = 0;
	char inbuff[6];

		count = write(stream, _SIZE, 6);
		if (count < 0) {
			printf("###### WRITE ERROR ######\nEXITING\n");
			exit(1);
		}
	for (int i = 0; i < 60; i++) {
		count = read(stream, (void*) inbuff, 6);
		//FOR DEBUGGING
				for (int j = 0; j < 6; ++j) {
					printf("inbuff[%d] = 0x%X == SYNCREP = 0x%X\n", j, inbuff[j], _SIZE_ACK[j]);
				}
		if (count < 0) {
			printf("###### READ ERROR ######\nEXITING\n");
			exit(1);
		}

		if (inbuff[0] == _SIZE_ACK[0] && inbuff[1] == _SIZE_ACK[1] && inbuff[2] == _SIZE_ACK[2]) {
			printf("Camera Sized\n");
			return 1;
		}
	}
	return 0;
}


int camera_Jpg(int stream, char* str) {
	char _GET[] = {0xAA, 0x04, 0x05, 0x00, 0x00, 0x00};
	char _GET_ACK[] = {0x0AA, 0x0E, 0x04, 0x00, 0x00, 0x00};
	char _DATA[] = {0xAA, 0x0A, 0x05, 0x00, 0x00, 0x00};
	char _DACK[] = {0xAA, 0x0E, 0x00, 0x00, 0x00, 0x00};
	char received[512];
	char inbuff[6];
	FILE *img;
	int count;
	int size = 0;
	int temp = 0;
	int nump = 0;
  int yes = 0;
	int pcknum = 0;
	img = fopen(str, "wb");

	count = write(stream, _GET, 6);
	if (count < 0) {
		printf("###### WRITE STREAM ERROR ######\nEXITING\n");
		exit(1);
	}
	for (int i = 0; i < 60; i++) {
		count = read(stream, (void*) inbuff, 6);
		if (count < 0) {
			printf("###### READ ERROR ######\nEXITING\n");
			exit(1);
		}
		//FOR DEBUGGING
				for (int j = 0; j < 6; ++j) {
					printf("inbuff[%d] = 0x%X == SYNCREP = 0x%X\n", j, inbuff[j], _GET_ACK[j]);
				}
				if (inbuff[0] == _GET_ACK[0] && inbuff[1] == _GET_ACK[1] && inbuff[2] == _GET_ACK[2]) {
					i = 60;
					printf("getacked\n");
				}
		}
	for(int i = 0; i < 60; i++) {
		count = read(stream, (void*) inbuff, 6);
		//FOR DEBUGGING
				for (int j = 0; j < 6; ++j) {
					printf("inbuff[%d] = 0x%X == SYNCREP = 0x%X\n", j, inbuff[j], _DATA[j]);
				}
				if (count < 0) {
					printf("###### READ ERROR ######\nEXITING\n");
					exit(1);
				}
				printf("pre data\n");
				if (inbuff[0] == _DATA[0] && inbuff[1] == _DATA[1] && inbuff[2] == _DATA[2]) {
					i = 60;
					yes = 1;
				}
			}
			if(yes == 1) {
				printf("into the data\n");
				size += inbuff[5];
				temp = inbuff[4];
				temp = temp * 256;
				size += temp;
				temp = inbuff[3] ;
				temp = temp * 65536;
				size += temp;
				printf("size = %d\n", size);
				nump = size;

				count = write(stream, _DACK, 6);
				if (count < 0) {
					printf("###### WRITE FILE ERROR ######\nEXITING\n");
					exit(1);
				}

				count = read(stream, (void*) received, 512);
				if (count < 0) {
					printf("###### READ ERROR ######\nEXITING\n");
					exit(1);
				}
				size = received[4];
				temp = received[3];
				temp *= 256;
				size += temp;
				_DACK[4] = received[0];
				_DACK[5] = received[1];
				pcknum = 0;
				while (pcknum < nump) {
					char tempr[506];
					int j = 0;
					for(int k = 4; k < 510; k++) {
						tempr[j] = received[k];
						j++;
					}

					if ((fwrite(&tempr, sizeof(tempr), 1, img) <= 0)) {
						printf("###### WRITE FILE ERROR ######\nEXITING\n");
						exit(1);
					}
					_DACK[4] = received[0];
					_DACK[5] = received[1];
					write(stream, _DACK, 6);
					count = read(stream, (void*) received, 512);
					if (count < 0) {
						printf("###### READ ERROR ######\nEXITING\n");
						exit(1);
					}
					temp = received[3];
					temp *= 256;
					pcknum = temp;
					printf("%d, %d\n", pcknum, nump);
				}
		return 1;
	}
printf("failed.\n");
return 0;
}
