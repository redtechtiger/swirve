#ifndef MODULE_H
#define MODULE_H

#include "handler.hpp"
#include "archive.hpp"
#include <string>

#define RELATIVESERVERLOCATION "/data/serverdata/"

class ServerModule : public MinecraftHandler {
    private:

    public:
		ServerModule(Archive _archive);
		void Configure(Archive _archive);

		std::string Name;
		std::string LaunchPath;
		std::string RealPath;
		unsigned long ID;
		int AssignedPort;
		int RamAllocated;
		int JavaVersion;
		std::vector<unsigned long> AccessIDs;

};	

#endif
