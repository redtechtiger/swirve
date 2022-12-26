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
        int forkId;
    public:
        int killFork();
        int tryStop();
	int isAlive();

	void setInput(const char* _writeBuffer, unsigned long _len);
        std::string getOutput();

	int executeJarAsync(char* _binary);
        int executeJarAsync(char* _binary, char* _env);
};

#endif
