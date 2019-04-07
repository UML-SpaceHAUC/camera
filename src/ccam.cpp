#include "ccam.h"

ccam::ccam( bool debug, std::string file ) {
    
}

bool ccam::setup( std::string stream ) {
    stream_ = open( stream.c_str(), O_RDWR | O_NOCTTY | O_NDELAY );
    if ( stream_ < 0 ) {
        if( debug_ ) {
            perror( "Unable to open UART" );
        }
        return false;
    }
    struct termios options;
    tcgetattr( stream_, &options );
    options.c_cflag = B115200 | CS8 | CLOCAL | CREAD;
    options.c_iflag = IGNPAR;
    options.c_oflag = 0;
    options.c_iflag = 0;
    tcflust( stream_, TCIFLUSH );
    tcsetattr( stream_, TCSANOW, &options );
}

bool ccam::takePicture() {
    if ( sync() && initialize() && cameraSize() && cameraJpg() ) {
        return true;
    } else {
        return false;
    }
}

bool ccam::sync() {
    char inbuff[6];
    int count = 0;
    unsigned time = 5000;

    for ( unsigned i = 0; i < 60; ++i ) {
        count = write( stream_, _SYNC_COMMAND, 6 );
        if ( count < 0 ) {
            if ( debug_ ) {
                perror( "failed to write to stream" )
            }
            exit( 1 );
        }
        count = read( stream_, _SYNC_COMMAND, 6 );
        if ( count < 0 ) {
            if ( debug_ ) {
                perror( "Failed to read from stream" );
            }
            exit( 1 );
        }

        
        // DEBUG: Compares what you receive with what you expect
        if (debug_) {
            for (unsigned j = 0; j < 6; ++j) {
                printf("inbuff[%d] = 0x%X == SYNCREP = 0x%X\n", j, inbuff[j],
                    _SYNC_ACK_REPLY[j]);
            }
        }
        // ENDDEBUG

        if ( inbuff[0] == _SYNC_ACK_REPLY[0] && inbuff[1] ==
             _SYNC_ACK_REPLY[1] && inbuff[2] == _SYNC_ACK_REPLY[2] ) {
            if ( debug ) {
                printf( "We got ack!! \n Checking for SYNC...\n" );
            }
            count = read( stream_, (void*) inbuff, 6 );
            if ( count < 0 ) {
                if ( debug_ ) {
                    printf("###### READ ERROR ######\nEXITING\n");
                    exit(1);
                }
                if ( inbuff[0] == _SYNC_ACK_REPLY_EXT[0] && inbuff[1] ==
                    _SYNC_ACK_REPLY_EXT[1] ) {
                if ( debug_ ) {
                    printf( "We got SYNC!!\n Sending ack...\n" );
                }
                count = write( stream_, _SYNC_ACK_REPLY, 6 );
                if ( count < 0 ) {
                    if ( debug_ ) {
                         printf("###### WRITE ERROR ######\nRETRYING\n");
                    }
                } else {
                    if ( debug_ ) {
                        printf("Successfull SYNC!\n");
                    }
                    i = 60;
                    sleep(2); // As specified in datasheet
                    return true;
                }
            }
        }
        time += 1000;
        usleep( time );
    }
    return false;
}

bool ccam::initialize() {
    int count = 0;
    char inbuff[6];

    count = write( stream_, _CAMERA_INIT, 6 );
    if ( count < 0 ) {
        if ( debug_ ) {
            printf("###### WRITE ERROR ######\nEXITING\n");
        }
        exit(1);
    }
    usleep(50000);
    count = read( stream_, (void*) inbuff, 6 );
    if (debug_) {
        for ( unsigned j = 0; j < 6; ++j ) {
            printf("inbuff[%d] = 0x%X == SYNCREP = 0x%X\n", j, inbuff[j],
                _CAMERA_INIT_ACK[j]);
        }
    }
    if (count < 0) {
        if (debug_) {
            printf("###### READ ERROR ######\nEXITING\n");
        }
        exit(1);
    }
    if (inbuff[0] == _CAMERA_INIT_ACK[0] && inbuff[1] ==
        _CAMERA_INIT_ACK[1] && inbuff[2] == _CAMERA_INIT_ACK[2]) {
        if (debug_) {
            printf("Camera Initialized\n");
        }
        return true;
    }
    return false;
}

bool ccam::cameraSize() {
    /* This function sets the package size of JPEG images to
    512 bytes. For other configurations, refer to the
    USERGUIDE.md and the datasheet.
    */
    int count = 0;
    char inbuff[6];

    count = write( stream_, _SIZE, 6 );
    if (count < 0) {
        if (debug_) {
            printf("###### WRITE ERROR ######\nEXITING\n");
        }
        exit(1);
    }
    usleep(50000); // Hardware restriction
    count = read(stream_, (void*) inbuff, 6);
    if (debug_) {
        for (int j = 0; j < 6; ++j) {
            printf("inbuff[%d] = 0x%X == SYNCREP = 0x%X\n",
                j, inbuff[j], _SIZE_ACK[j]);
        }
    }
    if (count < 0) {
        if (debug_) {
            printf("###### READ ERROR ######\nEXITING\n");
        }
        exit(1);
    }
    if (inbuff[0] == _SIZE_ACK[0] && inbuff[1] == _SIZE_ACK[1]
        && inbuff[2] == _SIZE_ACK[2]) {
        if (debug_)
            printf("Camera Sized\n");
        return true;
    }
    return false;
}

int cameraJpg(int stream, char* str, int debug) {
    /* This function retrieves the JPEG from the camera. It is currently
    setup for siz 512 byte packages, and may not work for smaller sizes.
    Refer to USERGUIDE.md and the datasheet for further details.
    */
    char received[512];
    char inbuff[6];
    FILE *img;
    int count;
    int pcknum = 1;
    int size = 0;
    int temp = 0;
    img = fopen(outFile_.c_str(), "wb");

    count = write(stream_, _GET, 6);
    if (count < 0) {
        if (debug_)
            printf("###### WRITE STREAM ERROR ######\nEXITING\n");
        exit(1);
    }
    usleep(80000);
    count = read(stream_, (void*) inbuff, 6);
    if (count < 0) {
        if (debug_)
            printf("###### READ ERROR ######\nEXITING\n");
        exit(1);
    }
    if (debug_) {
        for (int j = 0; j < 6; ++j) {
            printf("inbuff[%d] = 0x%X == GET_ACK = 0x%X\n",
                j, inbuff[j], _GET_ACK[j]);
        }
    }
    if (inbuff[0] == _GET_ACK[0] && inbuff[1] == _GET_ACK[1]
        && inbuff[2] == _GET_ACK[2]) {
        usleep(50000);
        count = read(stream_, (void*) inbuff, 6);
        if (debug_) {
            for (int j = 0; j < 6; ++j) {
                printf("inbuff[%d] = 0x%X == SYNCREP = 0x%X\n", j, inbuff[j], _DATA[j]);
            }
        }
        if (count < 0) {
            if (debug_)
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
            if (debug_)
                printf("size = %d\n", size); //FILESIZE in bytes
            count = write(stream_, _DACK, 6);
            if (count < 0) {
                if (debug_)
                    printf("###### WRITE FILE ERROR ######\nEXITING\n");
                exit(1);
            }
            usleep(50000);
            count = read(stream_, (void*) received, 512);
            if (count < 0) {
                if (debug_)
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
                    if (debug_)
                        printf("###### WRITE FILE ERROR ######\nEXITING\n");
                    exit(1);
                }
                if (write(stream_, _DACK, 6) < 0) {
                    if (debug_)
                        printf("Failed to Write.");
                    exit(1);
                }
                usleep(50000);
                count = read(stream_, (void*) received, 512);
                _DACK[4] = received[0];
                _DACK[5] = received[1];
                if (debug_)
                    printf("pckgsize = %d\n", count);
                pcknum = count;
                if (count < 0) {
                    if (debug_)
                        printf("###### READ ERROR ######\nEXITING\n");
                    exit(1);
                }
            }
            if (write(stream_, _DACK, 6) < 0) {
                if (debug_)
                    printf("Failed to Write.");
                exit(1);
            }
            fclose(img);
            return true;
        }
    }
    fclose(img);
    if (debug_)
        printf("failed.\n");
    return false;
}

ccam::_SYNC_COMMAND = { 0xAA, 0x0D, 0x00, 0x00, 0x00, 0x00 };
ccam::_SYNC_ACK_REPLY = { 0xAA, 0x0E, 0x0D, 0x00, 0x00, 0x00 };
ccam::_SYNC_ACK_PRELY_EXT = { 0xAA, 0x0D, 0x00, 0x00, 0x00, 0x00 };
ccam::_CAMERA_INIT = { 0xAA, 0x01, 0x00, 0x07, 0x03, 0x07 };
ccam::_CAMERA_INIT_ACK = { 0xAA, 0x0E, 0x01, 0x00, 0x00, 0x00 };
ccam::_SIZE = { 0xAA, 0x06, 0x08, 0x00, 0x02, 0x00 };
ccam::_SIZE_ACK = { 0xAA, 0x0E, 0x06, 0x00, 0x00, 0x00 };
ccam::_GET[] = { 0xAA, 0x04, 0x05, 0x00, 0x00, 0x00 };
ccam::_GET_ACK[] = { 0x0AA, 0x0E, 0x04, 0x00, 0x00, 0x00 };
ccam::_DATA[] = { 0xAA, 0x0A, 0x05, 0x00, 0x00, 0x00 };
ccam::_DACK[] = { 0xAA, 0x0E, 0x00, 0x00, 0x00, 0x00 };
