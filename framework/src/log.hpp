#ifndef LOG_H
#define LOG_H

#include <string>

struct flags {
    bool IGNOREINFO;
    bool IGNOREWARN;
    bool IGNOREFATAL;
};

class Logger {

	private:
		bool ignoreinfo;
		bool ignorewarn;
		bool ignorefatal;

    public:
		Logger();
		Logger(flags);

		int infofunc(const std::string pre, const std::string msg, const int ret);
		void infofuncstart(const std::string pre, const std::string msg);
		int infofuncreturn(const int ret);
		void info(const std::string pre, const std::string msg);
		void warn(const std::string pre, const std::string msg);
		void fatal(const std::string pre, const std::string msg);

};

#endif
