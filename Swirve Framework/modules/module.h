#include "../handler/handler.h"
#include "../archive/archive.h"
#ifndef MODULE_H
#define MODULE_H

#include <string>

class ServerModule {
    private:

    public:
	ServerModule(Archive _archive); // TODO: ADD SERIALIZATION DATA TYPE TO CONSTRUCTOR
	MinecraftHandler Handle;
	std::string Name;
	std::string LaunchPath;
	std::vector<unsigned long> AccessIDs;


};	

#endif
