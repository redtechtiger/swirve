#include <iostream>
#include "log.h"

void Logger::logFunctionFail(int _return, const char* _name) {
    if(_return==0) return;
    std::cout << "[LOGGER]: Function '" << _name << "' failed, with return code '" << _return << "'.\n";
}
void Logger::logFunctionFailDebug(int _return, const char* _name) {
    
}
void Logger::logFunction(int _return, const char* _name) {
    std::cout << "[LOGGER]: Function '" << _name << "' gave return code '" << _return << "'.\n";
}
void Logger::logFunctionDebug(int _return, const char* _name) {

}
void Logger::log(const char* _content) {
    std::cout << "[LOGGER]: " << _content << "\n";
}
void Logger::logDebug(const char* _content) {
    
}