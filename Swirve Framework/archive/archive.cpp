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
    pathStream << "./data/" << _id << ".arc";
    return pathStream.str();
}

int Archiver::writeIDVector(std::vector<unsigned long> _ids) {
    std::ofstream idStream;
    idStream.open("./data/config.arc");
    if(idStream.is_open()) {
	for(auto id : _ids) {
	    idStream << id << '[';
	}
	idStream.close();
	return 0;
    } else return -1;
}

int Archiver::EraseID(unsigned long _id) {
    std::vector<unsigned long> _buffer;
    if(LoadIDs(_buffer)!=0) return -1;
    for(int i=0;i<_buffer.size();i++) {
	if(_buffer[i]==_id) {
	    _buffer.erase(_buffer.begin()+i);
	}
    }
    return writeIDVector(_buffer);
}

int Archiver::PushID(unsigned long _id) {
    std::vector<unsigned long> _buffer;
    if(LoadIDs(_buffer)!=0) return -1;
    _buffer.push_back(_id);
    return writeIDVector(_buffer);
}

int Archiver::LoadIDs(std::vector<unsigned long> &_ids) {
    std::ifstream idStream;
    std::string buffer;
    
    idStream.open("./data/config.arc");
    if(idStream.is_open()) {
	while(getline(idStream,buffer,'[')) {
	    _ids.push_back(std::stoul(buffer));
	}
	idStream.close();
	return 0;
    } else return -1;
}

int Archiver::LoadArchive(unsigned long _id, Archive &_archive) {
    
    _archive.ID = _id;

    std::string path = getPath(_id);
    std::ifstream archiveStream;
    
    std::string buffer;
    std::vector<std::string> rawData;
    
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
}

int Archiver::GetNewHash(unsigned long& _buffer) {
    std::hash<std::string> hash_obj;
    sleep(1);
    _buffer = hash_obj(std::to_string(static_cast<long>(time(nullptr)))); // Generate hash from epoch
}
