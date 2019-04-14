#include <cstdarg>
#include <cstdio>
#include "ucam.h"

ucam::ucam( std::string outFile, va_list args ) {
    outFile_ = outFile;
    streamName_ = va_arg( args, char * );
    debug_ = va_arg( args, int );
}

ucam::~ucam() {
    if ( stream_ > 0 ) {
        close( stream_ );
    }
}

bool ucam::setup() {
    stream_ = open( streamName_.c_str(), O_RDWR | O_NOCTTY | O_NDELAY );
    if ( stream_ < 0 ) {
        error( "Unable to open UART" );
        return false;
    }
    struct termios options;
    tcgetattr( stream_, &options );
    options.c_cflag = B115200 | CS8 | CLOCAL | CREAD;
    options.c_iflag = IGNPAR;
    options.c_oflag = 0;
    options.c_iflag = 0;
    tcflush( stream_, TCIFLUSH );
    tcsetattr( stream_, TCSANOW, &options );
    return true;
}

std::string ucam::takePicture( std::string loc, bool debug ) {
    outFile_ = loc;
    debug_ = debug;
    if ( sync() && initialize() && cameraSize() && cameraJpg() ) {
        return outFile_;
    } else {
        return "";
    }
}

bool ucam::sync() {
    char inbuff[6];
    int count = 0;
    unsigned time = 5000;
    //std::cout << stream_;
    for ( unsigned i = 0; i < 60; ++i ) {
        count = write( stream_, _SYNC_COMMAND, 6 );
        if ( count < 0 ) {
            error( "Failed to write to stream" );
            return false;
        }
        sleep(1);
        count = read( stream_, inbuff, 6 );
        if ( count < 0 ) {
            error( "Failed to read from stream" );
            return false;
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
            debug( "We got ack!! \n Checking for SYNC...\n" );
            count = read( stream_, (void*) inbuff, 6 );
            if ( count < 0 ) {
                error("###### READ ERROR ######\nEXITING\n");
                return false;
            }
            if ( inbuff[0] == _SYNC_ACK_REPLY_EXT[0] && inbuff[1] ==
                _SYNC_ACK_REPLY_EXT[1] ) {
                debug( "We got SYNC!!\n Sending ack...\n" );
                count = write( stream_, _SYNC_ACK_REPLY, 6 );
                if ( count < 0 ) {
                    debug("###### WRITE ERROR ######\nRETRYING\n");
                } else {
                    debug("Successfull SYNC!\n");
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

bool ucam::initialize() {
    int count = 0;
    char inbuff[6];

    count = write( stream_, _CAMERA_INIT, 6 );
    if ( count < 0 ) {
        error("###### WRITE ERROR ######\nEXITING\n");
        return false;
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
        error("###### READ ERROR ######\nEXITING\n");
        return false;
    }
    if (inbuff[0] == _CAMERA_INIT_ACK[0] && inbuff[1] ==
        _CAMERA_INIT_ACK[1] && inbuff[2] == _CAMERA_INIT_ACK[2]) {
        debug("Camera Initialized\n");
        return true;
    }
    return false;
}

bool ucam::cameraSize() {
    /* This function sets the package size of JPEG images to
    512 bytes. For other configurations, refer to the
    USERGUIDE.md and the datasheet.
    */
    int count = 0;
    char inbuff[6];

    count = write( stream_, _SIZE, 6 );
    if ( count < 0 ) {
        error("###### WRITE ERROR ######\nEXITING\n");
        return false;
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
        error("###### READ ERROR ######\nEXITING\n");
        return false;
    }
    if (inbuff[0] == _SIZE_ACK[0] && inbuff[1] == _SIZE_ACK[1]
        && inbuff[2] == _SIZE_ACK[2]) {
        debug("Camera Sized\n");
        return true;
    }
    return false;
}

bool ucam::cameraJpg() {
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
        error("###### WRITE STREAM ERROR ######\nEXITING\n");
        fclose( img );
        return false;
    }
    usleep(80000);
    count = read(stream_, (void*) inbuff, 6);
    if (count < 0) {
        error("###### READ ERROR ######\nEXITING\n");
        fclose( img );
        return false;
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
            error("###### READ ERROR ######\nEXITING\n");
            fclose( img );
            return false;
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
            debug("size = " + std::to_string(size)); //FILESIZE in bytes
            count = write(stream_, _DACK, 6);
            if (count < 0) {
                error("###### WRITE FILE ERROR ######\nEXITING\n");
                fclose( img );
                return false;
            }
            usleep(50000);
            count = read(stream_, (void*) received, 512);
            if (count < 0) {
                error("###### READ ERROR ######\nEXITING\n");
                fclose( img );
                return false;
            }
            temp = received[3];
            size = received[4];
            temp *= 256;
            size += temp;
            _DACK[4] = received[0];
            _DACK[5] = received[1];
            while (pcknum > 0) {
                char tempr[506];
                int j = 0;
                for (int k = 4; k < 510; k++) {
                    tempr[j] = received[k];
                    j++;
                }
                count = fwrite(&tempr, 1, sizeof(tempr), img);
                if ((count <= 0)) {
                    error("###### WRITE FILE ERROR ######\nEXITING\n");
                    fclose( img );
                    return false;
                }
                if (write(stream_, _DACK, 6) < 0) {
                    error("Failed to Write.");
                    fclose( img );
                    return false;
                }
                usleep(50000);
                count = read(stream_, (void*) received, 512);
                _DACK[4] = received[0];
                _DACK[5] = received[1];
                debug("pckgsize = " +  std::to_string(count));
                pcknum = count;
                /*if (count < 0) {
                    error("###### READ ERROR ######\nEXITING\n");
                    fclose( img );
                    return false;
                }*/
            }
            if (write(stream_, _DACK, 6) < 0) {
                error("Failed to Write.");
                fclose( img );
                return false;
            }
            fclose(img);
            return true;
        }
    }
    fclose(img);
    error("failed.\n");
    return false;
}

char ucam::_SYNC_COMMAND[6] = { 0xAA, 0x0D, 0x00, 0x00, 0x00, 0x00 };
char ucam::_SYNC_ACK_REPLY[6] = { 0xAA, 0x0E, 0x0D, 0x00, 0x00, 0x00 };
char ucam::_SYNC_ACK_REPLY_EXT[6] = { 0xAA, 0x0D, 0x00, 0x00, 0x00, 0x00 };
char ucam::_CAMERA_INIT[6] = { (char)0xAA, (char)0x01, (char)0x00, (char)0x07, (char)0x03, (char)0x07 };
char ucam::_CAMERA_INIT_ACK[6] = { (char)0xAA, (char)0x0E, (char)0x01, (char)0x00, (char)0x00, (char)0x00 };
char ucam::_SIZE[6] = { (char)0xAA, (char)0x06, (char)0x08, (char)0x00, (char)0x02, (char)0x00 };
char ucam::_SIZE_ACK[6] = { (char)0xAA, (char)0x0E, (char)0x06, (char)0x00, (char)0x00, (char)0x00 };
char ucam::_GET[6] = { (char)0xAA, (char)0x04, (char)0x05, (char)0x00, (char)0x00, (char)0x00 };
char ucam::_GET_ACK[6] = { (char)0x0AA, (char)0x0E, (char)0x04, (char)0x00, (char)0x00, (char)0x00 };
char ucam::_DATA[6] = { (char)0xAA, (char)0x0A, (char)0x05, (char)0x00, (char)0x00, (char)0x00 };
char ucam::_DACK[6] = { (char)0xAA, (char)0x0E, (char)0x00, (char)0x00, (char)0x00, (char)0x00 };
