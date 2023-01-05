#ifndef LOG_H
#define LOG_H

#include <string>

class Logger {
    private:
	int state = 0;
    public:
	void setDebug();
	void unsetDebug();  

	void logFunction(const std::string msg, const int ret); 
	void logLengthyFunction(const std::string msg);
	void logFinish(const int ret);
};

#endif
