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
            std::shared_ptr< T > cam(new T( outFile, args ) );
            if( !cam->setup() ) {
                return nullptr;
            }
            return std::move( cam );
        }
        virtual ~Camera() = default;
        virtual std::string takePicture( std::string, bool) = 0;
        virtual bool setup() = 0;
        virtual std::string outFile() { return outFile_; }
    protected:
        std::string outFile_;

};

#endif  // INCLUDE_CAMERA_H
