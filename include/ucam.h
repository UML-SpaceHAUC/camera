// Copyright Sean Cummings 2019
#ifndef CCAM_UCAM_H_
#define CCAM_UCAM_H_

#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <termios.h>
#include <unistd.h>
#include "camera.h"

class ccam : public Camera {
    public:
        static ccam make( std::string file, std::string stream, bool debug );

        std::string takePicture();
    private:
        ccam( bool debug, std::string file ) : debug_( debug ), outFile_( file );
        bool setup( std::string stream );
        bool sync();
        bool configure( Resolution res, Camera::PictureType type );
        bool initialize();
        bool cameraSize();
        bool cameraJpg();

        static const char _SYNC_COMMAND[6];
        static const char _SYNC_ACK_REPLY[6];
        static const char _SYNC_ACK_PRELY_EXT[6];
        static const char _CAMERA_INIT[6];
        static const char _CAMERA_INIT_ACK[6];
        static const char _GET[6];
        static const char _GET_ACK[6];
        static const char _DATA[6];
        static const char _DACK[6];
};

#endif  // CCAM_UCAM_H_
