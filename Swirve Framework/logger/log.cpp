#include <iostream>
#include "log.h"

#define MSG_0 "\u001b[32mOK\u001b[37m"
#define MSG__ "\u001b[31mFAIL\u001b[37m"

using namespace std;

void Logger::setDebug() {
    state = 1;
}

void Logger::unsetDebug() {
    state = 0;
}

void Logger::logFunction(const string msg, const int ret) {
    cout << msg << " [" << (ret == 0 ? MSG_0 : MSG__) << "\n";
    fflush(stdout);
}

void Logger::logLengthyFunction(const string msg) {
    cout << "[*] " << msg << " ";
    fflush(stdout);
    logging = 1;
}

void Logger::logFinish(const int ret);
