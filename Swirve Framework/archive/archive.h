#ifndef ARCHIVER_H
#define ARCHIVER_H

#include <string>
#include <vector>
#include "../handler/handler.h"

struct Archive {
    long ID;
    std::string Name;
    std::string LaunchPath;
    std::vector<unsigned long> AccessIDs;
};

class Archiver {
    private:
	static std::string getPath(unsigned long _id);
    public:
	static int LoadArchive(unsigned long _id, Archive &_archive);
	static int SaveArchive(unsigned long _id, Archive &_archive);
	static int GetNewHash(unsigned long& _buffer);
};

#endif
