#ifndef HANDLER_H
#define HANDLER_H

#include <string>
#include "../loader/async_loader.h"

class MinecraftHandler {
    private:
        AsynchronousApplicationLoader instance;
        enum POWERACTION {START, STOP, KILL, RESTART};
        enum POWERSTATE {OFFLINE,STARTING,ONLINE,STOPPING,RESTARTING,KILLING,FAULT};
        POWERSTATE state = OFFLINE;
        std::string serverLog;
        int changePowerState(POWERACTION action);
        int stopserver();
        int startserver();
        int restartserver();
        int killserver();
    public:
        int Start();
        int Stop();
        int Restart();
        int Kill();
        int GetLog(std::string _buffer);
};

#endif