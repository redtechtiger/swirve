#include "async_loader.h"
#include <iostream>
#include <stdexcept>
#include <stdio.h>
#include <string>
#include <thread>
#include <unistd.h>
#include <sys/ioctl.h>
#include "../logger/log.h"

#define READ_BUFFER_SIZE 128

std::string AsynchronousApplicationLoader::getOutput() {
    std::string _output;
    char _readBuffer[READ_BUFFER_SIZE] = {0};
    read(pipe2[0],_readBuffer,READ_BUFFER_SIZE);
    do {
        _output += _readBuffer;
    } while(read(pipe2[0],_readBuffer,READ_BUFFER_SIZE)==READ_BUFFER_SIZE);
    return _output;
}

void AsynchronousApplicationLoader::setInput(const char *_writeBuffer, unsigned long _len) {
    int _in = write(pipe1[1],_writeBuffer,_len);
}

void AsynchronousApplicationLoader::executeShellAsync(const char* _binary, const char* _args) {
    pipe(pipe1);
    pipe(pipe2);
    int _forkid = fork();
    if(_forkid<0) {
        throw std::runtime_error("[FATAL] Forking failed");
    } else if(_forkid==0) { // Child
        dup2(pipe1[0],STDIN_FILENO); // Assign (duplicate) new pipes
        dup2(pipe2[1],STDOUT_FILENO);
        close(pipe1[1]); // Close pipe ends as no longer needed
        close(pipe2[0]);
        execlp(_binary,"",_args,NULL); // Let binary take control
        std::cerr << "[FATAL] Forked child thread exited unexpectedly. Terminate program ASAP to prevent further damage to the system." << std::endl;
        throw std::runtime_error("[FATAL] Forked child thread exited unexpectedly.");
    } else { // Parent
        close(pipe1[0]); // Close unused pipe ends
        close(pipe2[1]);
        int flags = 1;
        ioctl(pipe2[0],FIONBIO,&flags); // Enable non-blocking IO calls
    }
}

void AsynchronousApplicationLoader::executeShellAsync(const char* _binary, const char* _env, const char* _args) {
    pipe(pipe1);
    pipe(pipe2);
    int _forkid = fork();
    if(_forkid<0) {
        throw std::runtime_error("[FATAL] Forking failed");
    } else if(_forkid==0) { // Child
        dup2(pipe1[0],STDIN_FILENO); // Assign (duplicate) new pipes
        dup2(pipe2[1],STDOUT_FILENO);
        close(pipe1[1]); // Close pipe ends as no longer needed
        close(pipe2[0]);
        std::string _buffer = "";

        execle(_binary,"",_args,NULL,_env,NULL); // Let binary take control
        std::cerr << "[FATAL] Forked child thread exited unexpectedly. Terminate program ASAP to prevent further damage to the system." << std::endl;
        throw std::runtime_error("[FATAL] Forked child thread exited unexpectedly.");
    } else { // Parent
        close(pipe1[0]); // Close unused pipe ends
        close(pipe2[1]);
        int flags = 1;
        ioctl(pipe2[0],FIONBIO,&flags); // Enable non-blocking IO calls
    }
}