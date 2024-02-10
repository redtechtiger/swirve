#include <iostream>
#include <sstream>
#include "handler.hpp"
#include "sysutils.hpp"
#include "async_loader.hpp"
#include "unistd.h"

#define ONLINE_STR "Done"

using namespace std;

std::string MinecraftHandler::getdirectoryfromfile(std::string _file) {
    // Get position of last "/" (iterate backwards), and return substring
    int _index = 0;
    for(int i=_file.length()-1;i>=0;i--) {
	    if(_file[i]=='/') {
	        _index = i;
	        break;
	    }
    }
    if(_index==0) return "";
    return _file.substr(0,_index);
}

int MinecraftHandler::Config(std::string _jarPath, int _ramAllocate, int _javaVersion) {
    jarPath = _jarPath;
    envPath = getdirectoryfromfile(_jarPath);
    ramAllocate = _ramAllocate;
    javaVersion = _javaVersion;
    return 0;
}

EPowerState MinecraftHandler::State() {
    updateserverlog();
    switch(state) {
        case OFFLINE: {
          sysutil.StopSync();
          CpuUtilization = 0;
          MemoryVmSize = 0;
          MemoryVmRSS = 0;
          break;
        }

        case STARTING: {
            if(log.find(ONLINE_STR)!=std::string::npos) {
                state = ONLINE;
            }
            if(instance.isAlive()!=1) {
                state = FAULT;
            }
            break;
        }

        case ONLINE: {
            if(instance.isAlive()!=1) {
                state = FAULT;
            }
            break;
        }

        case STOPPING: {
            if(instance.isAlive()!=1) {
                state = OFFLINE;
            }
            break;
        }

        case RESTARTING: {
            if(log.find(ONLINE_STR)!=std::string::npos&&rebootStage==1) {
                state = ONLINE;
            }
            break;
        }

        case KILLING: {
            if(instance.isAlive()!=1) {
                state = OFFLINE;
            }
            break;
        }

        case FAULT: {
          sysutil.StopSync();
          CpuUtilization = 0;
          MemoryVmSize = 0;
          MemoryVmRSS = 0;
          break;
        }
    }
    return state;
}

MinecraftHandler::~MinecraftHandler() {
    haltReboot();
}

void MinecraftHandler::startReboot() {
  Logger l;
  l.info("Handler", "Halting reboot...");
  if(rebootThread.joinable()) rebootThread.join();
  state = RESTARTING;
  rebootStage = 0;
  l.info("Handler", "Launching reboot thread!");
  rebootThread = thread(&MinecraftHandler::syncreboot, this);
}

void MinecraftHandler::haltReboot() {
    if(rebootThread.joinable()) pthread_cancel(rebootThread.native_handle());
}

void MinecraftHandler::syncreboot() {
    while(true) {
        sleep(1);
        if(instance.isAlive()==0) {
            rebootStage = 1;
            int ret = startserver();
            if(0>ret) {
              state = FAULT;
            }
            break;
        }
    }
}

int MinecraftHandler::resetserverlog() {
    log = "";
    return 0;
}

int MinecraftHandler::updateserverlog() {
    if(instance.isAlive()!=1) {
        return -1;
    }
    log += instance.getOutput();
    return 0;
}

int MinecraftHandler::startserver() {
    sysutil.StopSync();
    resetserverlog();
    std::stringstream ramarg;
    ramarg << "-Xmx" << ramAllocate << "G";
    std::vector<std::string> args{ramarg.str()};
    int returnval = instance.executeJarAsync(jarPath, args, envPath, javaVersion);
    if(returnval!=0) return returnval;
    sysutil.StartAsyncCPU(instance.forkId, CpuUtilization);
    sysutil.StartAsyncVmRSS(instance.forkId, MemoryVmRSS);
    sysutil.StartAsyncVmSize(instance.forkId, MemoryVmSize);
    return 0;
}

int MinecraftHandler::stopserver() {
    sysutil.StopSync();
    return instance.tryStop();
}

int MinecraftHandler::restartserver() {
    int _stopReturn = 0;
    _stopReturn = instance.tryStop();
    if(_stopReturn==0) {
        startReboot();
    }
    return _stopReturn;
}

int MinecraftHandler::killserver() {
    sysutil.StopSync();
    return instance.killFork();
}

int MinecraftHandler::changePowerState(EPowerAction action) {
    switch(action) {
        case START: {
            if(state==OFFLINE||state==FAULT) {
                state = STARTING;
                int returncode = startserver();
    		        if(returncode!=0) {
                    state = FAULT;
                    l.warn("Handler","Bad return value from AsyncLoader; couldn't start server");
                    return returncode;
                }
    		        return returncode;
            } else {
                return -1;
            }
        }

        case STOP: {
            if(state==ONLINE) {
                state = STOPPING;
		        int returncode = stopserver();
		        if(returncode!=0) {
                    state = FAULT;
                    return returncode;
                }
                return returncode;
            } else {
                return -1;
            }
        }

        case KILL: {
            if(state==ONLINE||state==STARTING||state==RESTARTING||state==STOPPING||state==FAULT) {
                state = KILLING;
		        int returncode = killserver();
		        if(returncode!=0) state = FAULT;
                return returncode;
            } else {
                return -1;
            }
        }

        case RESTART: {
            if(state==ONLINE) {
                state = RESTARTING;
		        int returncode = restartserver();
                if(returncode!=0) state = FAULT;
		        return returncode;
            } else {
                return -1;
            }
        }

	default:
	    return -1;
    }
}

int MinecraftHandler::Start() {
    return changePowerState(START);
}

int MinecraftHandler::Stop() {
    return changePowerState(STOP);
}

int MinecraftHandler::Restart() {
    return changePowerState(RESTART);
}

int MinecraftHandler::Kill() {
    return changePowerState(KILL);
}

int MinecraftHandler::SendCommand(std::string _buffer) {
    instance.setInput(_buffer.c_str(),_buffer.length());
    instance.setInput("\n",1);
    return 0;
}

int MinecraftHandler::GetLog(std::string& _buffer) {
    updateserverlog();
    _buffer = log;
    return 0;
}
