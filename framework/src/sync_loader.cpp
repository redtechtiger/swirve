#include "sync_loader.hpp"
#include <string>
#include <stdexcept>
#include <future>

int SynchronousApplicationLoader::executeShell(const char* _cmdLine, std::string& _buffer) {
    char _writeBuffer[128];
    int _status;
    FILE* pipe = popen(_cmdLine,"r");
    if(!pipe) throw std::runtime_error("[SYNCLOADER] [FATAL] POPEN FUNCTION FAIL");
    try {
        while(fgets(_writeBuffer, sizeof _writeBuffer, pipe)!=NULL) {
            _buffer += _writeBuffer;
        }
    } catch (...) {
        pclose(pipe);
        throw;
    }
    _status = pclose(pipe);
    return _status/256;
}
