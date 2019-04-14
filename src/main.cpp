// Copyright 2019 Sean Cummings
#include <string>
#include <memory>
#include <iostream>
#include <OctopOS/publisher.h>
#include <OctopOS/subscriber.h>
#include "camera.h"
#include "mock.h"
#include "ucam.h"

typedef struct {
    bool takePicture;
    bool debug;
} CameraType;

int main(int argc, char* argv[]) {
    key_t octoKey = std::stoi(argv[1]);
    publisher< CameraType > cameraPublisher( "CAMERA_DATA", octoKey);
    subscriber< CameraType > cameraSubscriber( "CAMERA_DATA", octoKey );
    publisher< bool > cameraLive( "CAMERA_LIVE", octoKey );
    
    auto camera = Camera::make< ucam >( "", "/dev/ttyS0", false );

    // 0. get_data_async
    // CDH_MAIN kill waiters
    cameraLive.publish( true );

    std::string location;
    for(;;) {
        CameraType command = cameraSubscriber.get_data();
        if ( command.takePicture == false ) {
            continue;
        }
        location = camera->takePicture( "/home/pi/plswork.jpg", command.debug ) ;
        /*if ( strcmp( (camera->outFile().c_str()), command.imgLocation ) ) {
            std::cerr << "Did not save file to correct location" << std::endl;
            memcpy( command.imgLocation, camera->outFile().c_str(), camera->outFile().size() );
        }*/
        command.takePicture = false;
        cameraPublisher.publish( command );
    }

    std::cerr << "HIT STRANGE STATE DIEING" << std::endl;
    cameraLive.publish( false );
    exit( -1 );
}
