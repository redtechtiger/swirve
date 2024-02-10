#ifndef SYNC_LOADER_H
#define SYNC_LOADER_H

#include <iostream>
#include <stdexcept>
#include <stdio.h>
#include <string>

class SynchronousApplicationLoader {
    private:
        std::string _bufferOut;
    public:
        static int executeShell(const char* _cmdLine, std::string& _buffer);
};

#endif