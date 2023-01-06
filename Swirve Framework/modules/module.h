#include "../handler/handler.h"
#include "../archive/archive.h"
#ifndef MODULE_H
#define MODULE_H

#include <string>

class ServerModule : public MinecraftHandler {
    private:

    public:
	ServerModule(Archive _archive); // TODO: ADD SERIALIZATION DATA TYPE TO CONSTRUCTOR
	std::string Name;
	std::string LaunchPath;
	unsigned long ID;
	int RamAllocated;
	std::vector<unsigned long> AccessIDs;


};	

#endif
