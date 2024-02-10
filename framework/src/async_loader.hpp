#ifndef ASYNC_LOADER_H
#define ASYNC_LOADER_H

#include <iostream>
#include <stdexcept>
#include <stdio.h>
#include <string>
#include <thread>
#include <vector>

class AsynchronousApplicationLoader {
    private:
        int pipe1[2];
        int pipe2[2];
        
    public:
        AsynchronousApplicationLoader();
        int killFork();
        int tryStop();
        int isAlive();
        int forkId = -1;

        void setInput(const char* _writeBuffer, unsigned long _len);
        std::string getOutput();

        int executeJarAsync(std::string _binary, std::vector<std::string> _args, std::string _env, int javaInt);
};

#endif
