// Copyright 2019 Sean Cummings
#include <string>
#include <memory>
#include <iostream>
#include "camera.h"
#include "mock.h"
#include "ucam.h"

typedef struct {
    bool takePicture;
    char imgLogcation[50];
} CameraType;

int main(int argc, char* argv[]) {
    std::string str = "outfile.jpg";
    int debug = 1;
    
    auto camera = Camera::make< ucam >( str, "/dev/ttyS0", debug );
//    auto camera = Camera::make< mock >( str );

    std::string location;
    if ( camera != nullptr ) {
        location = camera->takePicture();
    }

    if ( location != "" ) {
        std::cout << "Picture saved to: " << str << std::endl;
    } else {
        std::cerr << "Failed to take picture" << std::endl;
    }
}
