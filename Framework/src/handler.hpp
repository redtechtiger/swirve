#ifndef HANDLER_H
#define HANDLER_H

#include <string>
#include "async_loader.hpp"
#include "sysutils.hpp"
#include "log.hpp"
#include <filesystem>
#include <thread>

enum EPowerState {OFFLINE,STARTING,ONLINE,STOPPING,RESTARTING,KILLING,FAULT};
enum EPowerAction {START, STOP, KILL, RESTART};


class MinecraftHandler {

    private:
        AsynchronousApplicationLoader instance;
        EPowerState state = OFFLINE;
        std::string serverLog = "Swirve: Server ready to start";
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
        int javaVersion;
        SysUtils sysutil;

        Logger l;

        void haltReboot();
        void startReboot();
        void syncreboot();
        std::thread rebootThread;
        int rebootStage = 0;

    public:
        ~MinecraftHandler();

        EPowerState State();
	int Config(std::string _jarPath, int _ramAllocate, int _javaVersion);
	int Start();
        int Stop();
        int Restart();
        int Kill();
	int SendCommand(std::string _buffer);
        int GetLog(std::string& _buffer);

        int MemoryVmRSS = 0;
        int MemoryVmSize = 0;
        int CpuUtilization = 0;
};

#endif
