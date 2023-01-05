#include <iostream>
#include "log.h"

#define MSG_0 "\u001b[32mOK\u001b[0m"
#define MSG__ "\u001b[31mFAIL\u001b[0m"

using namespace std;

void Logger::setDebug() {
    state = 1;
}

void Logger::unsetDebug() {
    state = 0;
}

void Logger::logFunction(const string msg, const int ret) {
    cout << msg << " [" << (ret == 0 ? MSG_0 : MSG__) << "]\n";
    fflush(stdout);
}

void Logger::logLengthyFunction(const string msg) {
    cout << "[*] " << msg << " ";
    fflush(stdout);
}

void Logger::logFinish(const int ret) {
    cout << "[" << (ret == 0 ? MSG_0 : MSG__) << "]\n";
}
