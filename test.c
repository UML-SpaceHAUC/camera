#include <stdio.h>
#include <unistd.h>
#include <stdint.h>
#include <fcntl.h>
#include <termios.h>
#include <stdlib.h>

int camera_Sync(int stream);

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


	camera_Sync(stream);
}

int camera_Sync(int stream) {

	char _SYNC_COMMAND[6] = {0xAA, 0x0D, 0x00, 0x00, 0x00, 0x00};
	char _SYNC_ACK_REPLY[6] = {0xAA, 0x0E, 0x0D, 0x00, 0x00, 0x00};
	char _SYNC_ACK_REPLY_EXT[6] = {0xAA, 0x0D, 0x00, 0x00, 0x00, 0x00};
	char _SYNC_FINAL_COMMAND[6] = {0xAA, 0x0E, 0x00, 0x00, 0xF5, 0x00};

	char _INITIAL_COMMAND[6] = {0xAA, 0x01, 0x00, 0x07, 0x03, 0x07};
	char _GENERIC_ACK_REPLY[6] = {0xAA, 0x0E, 0x00, 0x00, 0x00, 0x00};
	char _PACK_SIZE[6] = {0xAA, 0x06, 0x08, 0x00, 0x02, 0x00};
	char _SNAPSHOT[6] = {0xAA, 0x05, 0x00, 0x00, 0x00, 0x00};
	char _GET_PICTURE[6] = {0xAA, 0x04, 0x01, 0x00, 0x00, 0x00};

	char inbuff[6];

	int i = 0;
	int count = 0;
	int j = 0;
	int k = 0;
	unsigned int time = 5000;
	for (i = 0; i < 60; i++) {
			count = write(stream, _SYNC_COMMAND, 6);
			if (count < 0)
				perror("can't count");

		count = read(stream, (void*) inbuff, 6);

		if (count < 0) {
			printf("###### READ ERROR ######\n");
		}

//FOR DEBUGGING
//		for (j = 0; j < 6; ++j) {
//			printf("inbuff[%d] = 0x%X == SYNCREP = 0x%X\n", j, inbuff[j], _SYNC_ACK_REPLY[j]);
//		}

		if (inbuff[0] == _SYNC_ACK_REPLY[0] && inbuff[1] == _SYNC_ACK_REPLY[1] && inbuff[2] == _SYNC_ACK_REPLY[2]) {
					printf("We got ack!! \n Checking for SYNC...\n");
					count = read(stream, (void*) inbuff, 6);
					if (inbuff[0] == _SYNC_ACK_REPLY_EXT[0] && inbuff[1] == _SYNC_ACK_REPLY_EXT[1]) {
						printf("We got SYNC!!\n Sending ack...\n");
						count = write(stream, _SYNC_ACK_REPLY, 6);
						if (count < 0) {
							printf("no write :(\n");
						} else {
							i = 60;
						}
					}
				}
		else {
			printf("ACK, but no SYNC...\n Retrying...\n");
		}
		time += 1000;
		usleep(time);
	}
	printf("Successfull SYNC!\n");
	return 1;
}
