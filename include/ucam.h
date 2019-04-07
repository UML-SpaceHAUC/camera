// Copyright Sean Cummings 2019
#ifndef CCAM_UCAM_H_
#define CCAM_UCAM_H_

#include <fcntl.h>
#include <termios.h>
#include <unistd.h>
#include <cstdlib>
#include <string>
#include <iostream>
#include <memory>
#include "camera.h"

class ucam : public Camera {
    public:
        std::string takePicture();
        ucam( std::string outFile, va_list args );
        ~ucam();
        bool setup();
    private:
        bool sync();
        bool initialize();
        bool cameraSize();
        bool cameraJpg();

        inline void debug( std::string msg ) {
            if ( debug_ ) {
                std::cout << "[DEBUG] " << msg << std::endl;
            }
        }
        inline void error( std::string msg ) {
            std::cerr << "[ERROR] " << msg << std::endl;
        }

        bool debug_ = false;
        std::string outFile_ = "";
        std::string streamName_ = "";
        int stream_ = 0;

        static char _SYNC_COMMAND[6];
        static char _SYNC_ACK_REPLY[6];
        static char _SYNC_ACK_REPLY_EXT[6];
        static char _CAMERA_INIT[6];
        static char _CAMERA_INIT_ACK[6];
        static char _SIZE[6];
        static char _SIZE_ACK[6];
        static char _GET[6];
        static char _GET_ACK[6];
        static char _DATA[6];
        static char _DACK[6];
};

#endif  // CCAM_UCAM_H_
