#ifndef HANDLER_H
#define HANDLER_H

#include <string>
#include "../loader/async_loader.h"

enum EPowerState {OFFLINE,STARTING,ONLINE,STOPPING,RESTARTING,KILLING,FAULT};
enum EPowerAction {START, STOP, KILL, RESTART};

class MinecraftHandler {
    private:
        AsynchronousApplicationLoader instance;
        EPowerState state = OFFLINE;
        std::string serverLog;
        int changePowerState(EPowerAction action);
        int stopserver();
        int startserver();
        int restartserver();
        int killserver();
	int updateserverlog();
	int resetserverlog();
	std::string getdirectoryfromfile(std::string _file);
	std::string jarPath;
	std::string envPath;
	std::string log;
	int ramAllocate;
    public:
        EPowerState State();
	int Config(std::string _jarPath, int _ramAllocate);
	int Start();
        int Stop();
        int Restart();
        int Kill();
	int SendCommand(std::string _buffer);
        int GetLog(std::string& _buffer);
};

#endif
