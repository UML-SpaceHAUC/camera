#ifndef INCLUDE_MOCK_H
#define INCLUDE_MOCK_H

#include <cstdarg>
#include <string>
#include <memory>

class mock {
    public:
        mock( std::string file, va_list args ) : outFile_(file) {};
        std::string takePicture() { return outFile_; };
        bool setup() { return true; }
    private:
        std::string outFile_ = "";
};

#endif  // INCLUDE_MOCK_H
