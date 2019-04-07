#ifndef INCLUDE_CAMERA_H
#define INCLUDE_CAMERA_H

class Camera {
    public:
        template< typename T >
        static T make< T >( bool debug=false ) {
            T cam = T::make( debug );
            return std::move( cam );
        }
    protected:
        bool initialize( int stream, bool debug ) = 0;

};
