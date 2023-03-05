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
#include <sstream>
#include "../logger/log.h"

#define READ_BUFFER_SIZE 4096
#define JAVA_BIN "java"
#define BASH_BIN "bash"

template <typename T>
void clearArray(T& _buffer, int _len) {
    for(int i=0;i<_len-1;i++) {
        _buffer[i] = 0;
    }
}

AsynchronousApplicationLoader::AsynchronousApplicationLoader() { // Constructor!!
    signal(SIGPIPE, SIG_IGN);
}

int AsynchronousApplicationLoader::killFork() {
    return kill(forkId,SIGKILL);
}

int AsynchronousApplicationLoader::isAlive() {
    int status;
    return (waitpid(-1,&status,WNOHANG)>=0 ? 1 : 0);
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

int AsynchronousApplicationLoader::executeJarAsync(std::string _binary, std::vector<std::string> _args) {
    pipe(pipe1);
    pipe(pipe2);

    std::vector<std::string> cmdLine{JAVA_BIN}; // Downwards: Create "correct" argument list from a vector
    for(const auto& i : _args) {
	cmdLine.push_back(i);
    }
    cmdLine.push_back("-jar");
    cmdLine.push_back(_binary);

    std::vector<const char*> argv;
    for(const auto& s : cmdLine) {
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
        execvp(JAVA_BIN,const_cast<char* const*>(argv.data())); // Let binary take control
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

int AsynchronousApplicationLoader::executeJarAsync(std::string _binary, std::vector<std::string> _args, std::string _env) {
    pipe(pipe1);
    pipe(pipe2);

    // Parse input file and decide if it shall be executed by bash or java
    std::stringstream filenameStream(_binary);
    std::string buffer;
    while(getline(filenameStream, buffer, '.')) {
    }

    std::cout << "AsyncLoader: Detected file extension: " << buffer;
    bool isShell = false;
    if(buffer=="sh") {
        isShell = true;
        std::cout << "AsyncLoader: Warning: Running shell file! Several features will be disabled.";
    }

    std::string binaryExecute = isShell == true ? BASH_BIN : JAVA_BIN;

    std::vector<std::string> cmdLine{binaryExecute}; // Downwards: Create "correct" argument list from a vector

    if(!isShell) {							
        for (const auto& i : _args) {
        cmdLine.push_back(i);
        }
        cmdLine.push_back("-jar");
        cmdLine.push_back(_binary);
    } else {
        cmdLine.push_back(_binary);
    }

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
	close(pipe1[1]); // Close pipe ends as no longer needed.
        close(pipe2[0]);
        std::cerr << chdir(_env.c_str());
        std::cerr << execvp(binaryExecute.c_str(),const_cast<char* const*>(argv.data())); // Let binary take control
        std::cerr << "[FATAL] Forked child thread exited unexpectedly. Terminate program ASAP to prevent further damage to the system." << std::endl;
        throw std::runtime_error("[FATAL] Forked child thread exited unexpectedly.");
    } else { // Parent
        close(pipe1[0]); // Close unused pipe ends
        close(pipe2[1]);
        int flags = 1;
        ioctl(pipe2[0],FIONBIO,&flags); // Enable non-blocking IO calls.
        forkId = _forkid;
	return 0;
    }
}
