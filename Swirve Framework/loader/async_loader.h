#ifndef ASYNC_LOADER_H
#define ASYNC_LOADER_H

#include <iostream>
#include <stdexcept>
#include <stdio.h>
#include <string>
#include <thread>

class AsynchronousApplicationLoader {
    private:
        int pipe1[2];
        int pipe2[2];
    public:
        std::string getOutput();
        void setInput(const char* _writeBuffer, unsigned long _len);
        void executeShellAsync(const char* _binary, const char* _args);
        void executeShellAsync(const char* _binary, const char* _env, const char* _args);
        void waitForExit();
};

#endif