#include <iostream>
#include "log.hpp"
#include <unistd.h>
#include <iomanip>

#define MSG_0 "\u001b[32m OK \u001b[0m"
#define MSG_E "\u001b[31mFAIL\u001b[0m"
#define ASCIIGREEN "\u001b[32m"
#define ASCIICYAN "\u001b[36m"
#define ASCIIYELLOW "\u001b[33m"
#define ASCIIRED "\u001b[31m"
#define ASCIIRESET "\u001b[0m"

#define MSGLEN 60

using namespace std;

Logger::Logger(flags setflags) {
    ignorefatal = setflags.IGNOREFATAL;
    ignorewarn = setflags.IGNOREWARN;
    ignoreinfo = setflags.IGNOREINFO;
}

Logger::Logger() {

}

void Logger::info(const string pre, const string msg) {
    cout << ASCIICYAN << pre << ASCIIRESET << "-> " << msg << endl;
    fflush(stdout);
}

void Logger::warn(const string pre, const string msg) {
    cout << ASCIIYELLOW << pre << ASCIIRESET << "-> " << msg << endl;
    fflush(stdout);
}

void Logger::fatal(const string pre, const string msg) {
    cout << ASCIIRED << pre << "[FATAL]" << ASCIIRESET << "-> " << msg << endl;
    fflush(stdout);
}

int Logger::infofunc(const string pre, const string msg, const int ret) {
    std::cout << ASCIICYAN << pre << ASCIIRESET << "=> " << msg;
    cout << " " << (ret == 0 ? MSG_0 : MSG_E) << "\n";
    fflush(stdout);
    return ret;
}

void Logger::infofuncstart(const string pre, const string msg) {
    cout << ASCIICYAN << pre << ASCIIRESET << "=> " << msg;
    fflush(stdout);
}

int Logger::infofuncreturn(const int ret) {
    cout << " " << (ret == 0 ? MSG_0 : MSG_E) << "\n";
    fflush(stdout);
    return ret;
}
