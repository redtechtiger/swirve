#include <iostream>
#include "handler.h"
#include "../loader/async_loader.h"

int MinecraftHandler::startserver() {
    return instance.executeJarAsync("java");
}

int MinecraftHandler::stopserver() {
    return instance.tryStop();
}

int MinecraftHandler::restartserver() {
    if(instance.tryStop()==0) {
        return startserver();
    } else return -1;
    
}

int MinecraftHandler::killserver() {
    instance.killFork();
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
    }
}

int MinecraftHandler::Start() {
    return changePowerState(START);
}

int MinecraftHandler::Stop() {
    return changePowerState(STOP);
}

int MinecraftHandler::GetLog(std::string _buffer) {

}