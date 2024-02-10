#ifndef ARCHIVER_H
#define ARCHIVER_H

// Defines
#define ARCHIVECONFIGPATH "./data/archiveconfig.arc"
#define ARCHIVEDATAPATH "./data/archivedata."
#define ARCHIVEDATAEXT ".arc"

#define PORT_LOW 25500
#define PORT_HIGH 25600

// Types
#include <string>
#include <vector>

// Handler
#include "handler.hpp"

struct Archive {
    unsigned long ID;
    std::string Name;
    std::string LaunchPath;
    int Ram;
	int Port;
	int Java;
    std::vector<unsigned long> AccessIDs;
};

class Archiver {
    private:
    	static std::string getPath(unsigned long _id);
    	static int writeIDVector(std::vector<unsigned long> _ids);
    public:
    	static int LoadIDs(std::vector<unsigned long> &_ids);
    	static int PushID(unsigned long _id);
    	static int EraseID(unsigned long _id);
    	static int LoadArchive(unsigned long _id, Archive &_archive);
    	static int SaveArchive(unsigned long _id, Archive &_archive);
    	static int GetNewHash(unsigned long& _buffer);
    	static int GetNextPort();
};

#endif
