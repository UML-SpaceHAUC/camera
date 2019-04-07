// Copyright 2019 Sean Cummings
#include "camera.h"

int main(int argc, char* argv[]) {
    std::string str = "";
    int debug = 0;

    if (argv[2]) {
        debug = atoi(argv[2]);
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
    
    auto camera = Camera::make< ccam >( "/dev/tty0", str, debug );
    bool success = camera.takePicture();

    if ( success ) {
        std::cout << "Picture saved to: " << str << std::endl;
    } else {
        std::cerr << "Failed to take picture" << std::endl;
    }
}
