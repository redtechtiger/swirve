#include "async_loader.h"
#include <iostream>
#include <stdexcept>
#include <stdio.h>
#include <string>
#include <thread>
#include <unistd.h>
#include <sstream>
#include <vector>
#include <sys/wait.h>
#include <sys/types.h>
#include <signal.h>
#include <sys/ioctl.h>
#include "../logger/log.h"

#define READ_BUFFER_SIZE 4096

template <typename T>
void clearArray(T& _buffer, int _len) {
    for(int i=0;i<_len-1;i++) {
        _buffer[i] = 0;
    }
}

int AsynchronousApplicationLoader::killFork() {
    return kill(forkId,SIGKILL);
}

int AsynchronousApplicationLoader::isAlive() {
    int status;
    return waitpid(-1,&status,WNOHANG);
}

int AsynchronousApplicationLoader::tryStop() {
    setInput("stop\n",5);
    return 0;
}

std::string AsynchronousApplicationLoader::getOutput() {
    std::string _output;
    char _readBuffer[READ_BUFFER_SIZE] = {0};
    read(pipe2[0],_readBuffer,READ_BUFFER_SIZE);
    int longRead = 0;
    do {
        _output += _readBuffer;
        clearArray(_readBuffer,sizeof(_readBuffer));
    } while((longRead = read(pipe2[0],_readBuffer,READ_BUFFER_SIZE))==READ_BUFFER_SIZE);
    if(longRead>0) {
        _output += _readBuffer;
    }
    return _output;
}

void AsynchronousApplicationLoader::setInput(const char *_writeBuffer, unsigned long _len) {
    write(pipe1[1],_writeBuffer,_len);
}

int AsynchronousApplicationLoader::executeJarAsync(char* _binary) {
    pipe(pipe1);
    pipe(pipe2);

    const std::vector<std::string> cmdLine{"java","-jar","./forge-1.16.5-36.2.39.jar"}; // Downwards: Create "correct" argument list from a vector
    std::vector<const char*> argv;
    for (const auto& s : cmdLine) {
        argv.push_back(s.data());
    }
    argv.push_back(NULL);
    argv.shrink_to_fit();

    int _forkid = fork();
    if(_forkid<0) {
        throw std::runtime_error("[FATAL] Forking failed");
    } else if(_forkid==0) { // Child
        dup2(pipe1[0],STDIN_FILENO); // Assign (duplicate) new pipes
        dup2(pipe2[1],STDOUT_FILENO);
        close(pipe1[1]); // Close pipe ends as no longer needed
        close(pipe2[0]);
        execvp(_binary,const_cast<char* const*>(argv.data())); // Let binary take control
        std::cerr << "[FATAL] Forked child thread exited unexpectedly. Terminate program ASAP to prevent further damage to the system." << std::endl;
        throw std::runtime_error("[FATAL] Forked child thread exited unexpectedly.");
    } else { // Parent
        close(pipe1[0]); // Close unused pipe ends
        close(pipe2[1]);
        int flags = 1;
        //ioctl(pipe2[0],FIONBIO,&flags); // Enable non-blocking IO calls
        forkId = _forkid;
	return 0;
    }
}

int AsynchronousApplicationLoader::executeJarAsync(char* _binary, char* _env) {
    pipe(pipe1);
    pipe(pipe2);

    const std::vector<std::string> cmdLine{"java","-jar","./forge-1.16.5-36.2.39.jar"}; // Downwards: Create "correct" argument list from a vector
    std::vector<const char*> argv;
    for (const auto& s : cmdLine) {
        argv.push_back(s.data());
    }
    argv.push_back(NULL);
    argv.shrink_to_fit();

    int _forkid = fork();
    if(_forkid<0) {
        throw std::runtime_error("[FATAL] Forking failed");
    } else if(_forkid==0) { // Child
        dup2(pipe1[0],STDIN_FILENO); // Assign (duplicate) new pipes
        dup2(pipe2[1],STDOUT_FILENO);
        dup2(pipe2[1],STDERR_FILENO);
	close(pipe1[1]); // Close pipe ends as no longer needed
        close(pipe2[0]);
        std::cerr << chdir(_env);
        std::cerr << execvp(_binary,const_cast<char* const*>(argv.data())); // Let binary take control
        std::cerr << "[FATAL] Forked child thread exited unexpectedly. Terminate program ASAP to prevent further damage to the system." << std::endl;
        throw std::runtime_error("[FATAL] Forked child thread exited unexpectedly.");
    } else { // Parent
        close(pipe1[0]); // Close unused pipe ends
        close(pipe2[1]);
        int flags = 1;
        ioctl(pipe2[0],FIONBIO,&flags); // Enable non-blocking IO calls
        forkId = _forkid;
	return 0;
    }
}
