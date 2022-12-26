#include <iostream>/mnt/c/Users/jacaul/OneDrive - Täby Friskola/Skrivbordet/GitHub/Swirve-Userclient/Swirve-Userclient/Swir
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
	    if(1==0) {
		state = FAULT;
	    }
	    break;
	
	case ONLINE:
	    if(1==0) {
		state = FAULT;
	    }
	    break;
	
	case STOPPING:
	    if(1==0) {
		state = OFFLINE;
	    }
	    break;

	case RESTARTING:
	    if(log.find(ONLINE_STR)!=std::string::npos) {
	    	state = ONLINE;
	    }
	    break;
	
	case KILLING:
	    if(1==0) {
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
            if(state==OFFLINE) {
                state = STARTING;
                return startserver();
            } else {
                return -1;
            }

        case STOP:
            if(state==ONLINE) {
                state = STOPPING;
                return stopserver();
            } else {
                return -1;
            }

        case KILL:
            if(state==ONLINE||state==STARTING||state==RESTARTING) {
                state = KILLING;
                return killserver();
            } else {
                return -1;
            }

        case RESTART:
            if(state==ONLINE) {
                state = RESTARTING;
                return restartserver();
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

int MinecraftHandler::SendCommand(std::string _buffer) {
    instance.setInput(_buffer.c_str(),sizeof(_buffer.c_str()));
    return 0;
}

int MinecraftHandler::GetLog(std::string& _buffer) {
    updateserverlog();
    _buffer = log;
    return 0;
}
