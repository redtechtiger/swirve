#ifndef LOG_H
#define LOG_H

class Logger {
    public:
        static void logFunctionFail(int _return, const char* _name = "{undefined}");
        static void logFunctionFailDebug(int _return, const char* _name = "{undefined}");
        static void logFunction(int _return, const char* _name = "{undefined}");
        static void logFunctionDebug(int _return, const char* _name = "{undefined}");
        static void log(const char* _content);
        static void logDebug(const char* _content);
};

#endif