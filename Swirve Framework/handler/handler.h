#ifndef HANDLER_H
#define HANDLER_H

#include "../loader/async_loader.h"

class MinecraftHandler {
    private:
        AsynchronousApplicationLoader instance;
        enum {PRESETUP,STOPPED,STARTING,RESTARTING,KILLING,FAULT};
        void changePowerState();
    public:
        void Start();
        void Stop();
        void GetLog();
};

#endif