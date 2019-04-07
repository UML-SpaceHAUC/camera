#ifndef INCLUDE_CAMERA_H
#define INCLUDE_CAMERA_H

#include <cstdarg>
#include <string>
#include <memory>

class Camera {
    public:
        template< typename T >
        static std::shared_ptr< T > make( std::string outFile... ) {
            va_list args;
            va_start( args, outFile );
            std::shared_ptr< T > cam = std::shared_ptr< T >( new T( outFile, args ) );
            if( !cam->setup() ) {
                return nullptr;
            }
            return std::move( cam );
        }
        virtual std::string takePicture() = 0;
        virtual bool setup() = 0;
};

#endif  // INCLUDE_CAMERA_H
