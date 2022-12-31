#include <iostream>
#include <ctime>
#include <string>
#include <fstream>
#include <vector>
#include <sstream> // stringstream for string (path) concatenation
#include "archive.h"
#include <unistd.h>

std::string Archiver::getPath(unsigned long _id) {
    std::stringstream pathStream;
    pathStream << "./data/" << _id << ".bin";
    return pathStream.str();
}

int Archiver::LoadArchive(unsigned long _id, Archive &_archive) {
    std::string path = getPath(_id);
    std::ifstream archiveStream;
    
    std::string buffer;
    std::vector<std::string> rawData;
    
    std::cout << "Opening archive for reading at " << path << "\n";
    archiveStream.open(path);
    if(archiveStream.is_open()) {
	while(getline(archiveStream,buffer,'[')) {
	    rawData.push_back(buffer);
	}
	archiveStream.close();
	
	for(std::string i : rawData) {
	    switch(i[0]-'0') {
		case 0:
		    _archive.Name = i.substr(1); 
		    break;
		case 1:
		    _archive.LaunchPath = i.substr(1);
		    break;
		case 2:
		    _archive.AccessIDs.push_back(std::stoul(i.substr(1)));
		    break;
		default:
		    return -1;
	    }
	}
	return 0;
    } else return -1;
}

int Archiver::SaveArchive(unsigned long _id, Archive &_archive) {
    // Generate path
    std::string path = getPath(_id);
    std::ofstream archiveStream;
    
    archiveStream.open(path);
    if(archiveStream.is_open()) {
	archiveStream << "0" << _archive.Name << '[';
	archiveStream << "1" << _archive.LaunchPath << '[';
	for(auto id : _archive.AccessIDs) {
	    archiveStream << "2" << id << '[';
	}

	archiveStream.close();
	return 0;

    } else return -1;
    // archiveStream.open("./data/"+_id+".bin")
}

int Archiver::GetNewHash(unsigned long& _buffer) {
    std::hash<std::string> hash_obj;
    sleep(1);
    _buffer = hash_obj(std::to_string(static_cast<long>(time(nullptr)))); // Generate hash from epoch
}
