#include <iostream>
#include "handler.h"
#include "../loader/async_loader.h"

#define ONLINE_STR "Done"

POWERSTATE MinecraftHandler::State() {
    updateserverlog();
    switch(state) {
	case OFFLINE:
	    break;

	case STARTING:
	    if(log.find(ONLINE_STR)!=std::string::npos) {
		state = ONLINE;
	    }
	    if(instance.isAlive()!=0) {
		state = FAULT;
	    }
	    break;
	
	case ONLINE:
	    if(instance.isAlive()!=0) {
		state = FAULT;
	    }
	    break;
	
	case STOPPING:
	    if(instance.isAlive()!=0) {
		state = OFFLINE;
	    }
	    break;

	case RESTARTING:
	    if(log.find(ONLINE_STR)!=std::string::npos) {
	    	state = ONLINE;
	    }
	    break;
	
	case KILLING:
	    if(instance.isAlive()!=0) {
		state = OFFLINE;	
	    }
	    break;

	case FAULT:
	    break;
    }
    return state;
}

int MinecraftHandler::resetserverlog() {
    log = "";
}

int MinecraftHandler::updateserverlog() {
    log += instance.getOutput();
    return 0;
}

int MinecraftHandler::startserver() {
    resetserverlog();
    return instance.executeJarAsync("java","/mnt/c/Users/jacaul/OneDrive - Täby Friskola/Skrivbordet/GitHub/Swirve-Userclient/Swirve-Userclient/Swirve Framework/env");
}

int MinecraftHandler::stopserver() {
    return instance.tryStop();
}

int MinecraftHandler::restartserver() {
    int _stopReturn = 0;
    _stopReturn = instance.tryStop();
    if(_stopReturn==0) {
        return startserver();
    } else return _stopReturn;    
} 

int MinecraftHandler::killserver() {
    return instance.killFork();
}

int MinecraftHandler::changePowerState(POWERACTION action) {
    switch(action) {
        case START:
            if(state==OFFLINE||state==FAULT) {
                state = STARTING;
                int returncode = startserver();
		if(returncode!=0) state = FAULT;
		return returncode;
            } else {
                return -1;
            }

        case STOP:
            if(state==ONLINE) {
                state = STOPPING;
		int returncode = stopserver();
		if(returncode!=0) state = FAULT;
                return returncode;
            } else {
                return -1;
            }

        case KILL:
            if(state==ONLINE||state==STARTING||state==RESTARTING||state==STOPPING) {
                state = KILLING;
		int returncode = killserver();
		if(returncode!=0) {
		    state = FAULT;
		}
                return returncode;
            } else {
                return -1;
            }

        case RESTART:
            if(state==ONLINE) {
                state = RESTARTING;
		int returncode = restartserver();
                if(returncode!=0) state = FAULT;
		return returncode;
            } else {
                return -1;
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
