#ifndef LOG_H
#define LOG_H

#include <string>

class Logger {
    private:
	int state = 0;
	int logging = 0;
    public:
	static void setDebug();
	static void unsetDebug();  

	static void logFunction(const std::string msg, const int ret); 
	static void logLengthyFunction(const std::string msg);
	static void logFinish(const int ret);
};

#endif
