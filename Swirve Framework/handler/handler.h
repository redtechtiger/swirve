#ifndef HANDLER_H
#define HANDLER_H

#include <string>
#include "../loader/async_loader.h"

 enum POWERSTATE {OFFLINE,STARTING,ONLINE,STOPPING,RESTARTING,KILLING,FAULT};

class MinecraftHandler {
    private:
        AsynchronousApplicationLoader instance;
        enum POWERACTION {START, STOP, KILL, RESTART};
        POWERSTATE state = OFFLINE;
        std::string serverLog;
        int changePowerState(POWERACTION action);
        int stopserver();
        int startserver();
        int restartserver();
        int killserver();
	int updateserverlog();
	int resetserverlog();
	std::string log;
    public:
        POWERSTATE State();
	int Start();
        int Stop();
        int Restart();
        int Kill();
	int SendCommand(std::string _buffer);
        int GetLog(std::string& _buffer);
};

#endif
