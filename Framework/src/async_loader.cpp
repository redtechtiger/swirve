#include "async_loader.hpp"
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
#include <algorithm>
#include <sstream>
#include <string>
#include <limits.h>
#include "log.hpp"

#define READ_BUFFER_SIZE 4096
#define JAVA_PATH "/usr/lib/jvm"
#define BASH_BIN "bash"

using namespace std;

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
    if(forkId<0) return 0;
    int status;
    int result = waitpid(forkId, &status, WNOHANG)>=0 ? 1 : 0;
    if(result==0) forkId=-1;
    return result;
}

int AsynchronousApplicationLoader::tryStop() {
    setInput("stop\n",5);
    return 0;
}


// Error thrown here is stack-buffer-overflow using the AddressFSanitizer. _readBuffer overflows...?
std::string AsynchronousApplicationLoader::getOutput() {
    char buffer[1024] = { 0 };
    string out;
    while(0<read(pipe2[0], buffer, sizeof(buffer)-1)) {
        out += buffer;
    }
    isAlive();
    return buffer;
}

void AsynchronousApplicationLoader::setInput(const char *_writeBuffer, unsigned long _len) {
    isAlive();
    if(forkId<0) return;
    write(pipe1[1],_writeBuffer,_len);
}

int AsynchronousApplicationLoader::executeJarAsync(std::string _binary, std::vector<std::string> _args, std::string _env, int javaInt) {
    isAlive();
    if(forkId>0) return -1;
    pipe(pipe1);
    pipe(pipe2);

    int _forkid = fork();
    if(_forkid<0) {
        std::cerr << "AsyncLoader: FATAL: Forking failed, emergency stopping server boot";
        return -1;
    } else if(_forkid==0) { // Child

        dup2(pipe1[0],STDIN_FILENO); // Assign (duplicate) new pipes
        dup2(pipe2[1],STDOUT_FILENO);
        dup2(pipe2[1],STDERR_FILENO);
        close(pipe1[1]); // Close pipe ends as no longer needed.
        close(pipe2[0]);

        std::cout << "-----=====----- Swirve Framework AsyncLoader, v1.1-REV3 -----=====-----\n";
        std::cout << "[*] Successfully forked Framework process\n";
        std::cout << "Welcome to Swirve. Beginning bootup procedure...\n";

        // Find java executable
        if(0>chdir(JAVA_PATH)) {
          std::cout << "[*] Java configuration invalid\n";
          cout << "Swirve cannot continue bootup due to above reason(s). Powering down...\n";
          kill(getpid(), SIGTERM);
        };
        char javabuffer[PATH_MAX] = {0};
        FILE* pathSearcher = popen(((std::string)"find *java-"+std::to_string(javaInt)+(std::string)"* 2>/dev/null").c_str(), "r");
        fgets(javabuffer, sizeof(javabuffer), pathSearcher);
        std::string dirname = (std::string)javabuffer;
        dirname.erase(remove(dirname.begin(), dirname.end(), '\n'), dirname.end());
        std::string fulljavapath = (std::string)JAVA_PATH+(std::string)"/"+dirname+(std::string)"/bin/java";
        bool javaInvalid = false;

        if(dirname.empty()) {
            javaInvalid = true;
        }

        // Parse input file and decide if it shall be executed by bash or java
        std::stringstream filenameStream(_binary);
        std::string buffer;
        while(getline(filenameStream, buffer, '.')) { }

        bool isShell = false;
        if(buffer=="sh") {
            isShell = true;
        } else if(javaInvalid) {
            cout << "[*] Java installation invalid\n";
            cout << "Swirve cannot continue bootup due to above reason(s). Powering down...\n";
            kill(getpid(), SIGTERM);
        }
        std::string binaryExecute = isShell == true ? BASH_BIN : fulljavapath;
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

        std::cout << "[*] Computed launch type: " << (isShell == true ? "[BASH_SHELL]":"[JAVA_BIN]") << std::endl;
        std::cout << "[*] Launch target: " << binaryExecute.c_str() << endl;
        if(isShell) {
            std::cout << "[*] Warning! Swirve will attempt to launch the server with the BASH interpreter. This will mean certain features will be disabled and it is therefore up to you that the configuration is correct." << std::endl;
        }
        if(chdir(_env.c_str())!=0) {
            cout << "[*] Working directory invalid\n";
            cout << "Swirve cannot continue bootup due to above reason(s). Powering down...\n";
            kill(getpid(), SIGTERM);
        }
        std::cout << "Swirve has prepared for server boot and will soon hand over control to the server.\n------------------------------------------------------------------------\n";
        std::cerr << execvp(binaryExecute.c_str(),const_cast<char* const*>(argv.data())); // Let binary take control
        cout << "[*] Binary target invalid";
        cout << "Swirve cannot continue bootup due to above reason(s). Powering down...";
        kill(getpid(), SIGTERM);
    } else { // Parent
        close(pipe1[0]); // Close unused pipe ends
        close(pipe2[1]);
        int flags = 1;
        ioctl(pipe2[0],FIONBIO,&flags); // Enable non-blocking IO calls.
        forkId = _forkid;
	    return 0;
    }
    return -1;
}
