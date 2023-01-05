#ifndef ARCHIVER_H
#define ARCHIVER_H

#include <string>
#include <vector>
#include "../handler/handler.h"

struct Archive {
    unsigned long ID;
    std::string Name;
    std::string LaunchPath;
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
};

#endif
