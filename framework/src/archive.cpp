#include <iostream>
#include <ctime>
#include <string>
#include <fstream>
#include <vector>
#include <sstream> // stringstream for string (path) concatenation
#include "archive.hpp"
#include <unistd.h>
#include <algorithm>

using namespace std;

string Archiver::getPath(unsigned long _id) {
    stringstream pathStream;
    pathStream << ARCHIVEDATAPATH << _id << ARCHIVEDATAEXT;
    return pathStream.str();
}

int Archiver::writeIDVector(vector<unsigned long> _ids) {
    ofstream idStream;
    idStream.open(ARCHIVECONFIGPATH);
    if(idStream.is_open()) {
	for(auto id : _ids) {
	    idStream << id << '[';
	}
	idStream.close();
	return 0;
    } else return -1;
}

int Archiver::GetNextPort() {
	vector<unsigned long> ids;
	vector<int> ports;
	LoadIDs(ids);
	for(const auto &id : ids) { // Get all ports currently stored by archives
		Archive archive;
		int ret = LoadArchive(id, archive);
		if(ret!=0) continue;
		ports.push_back(archive.Port);
	}
	for(auto port=PORT_LOW; port<=PORT_HIGH; port++) {
		cout << "Checking if port " << port << " is busy..." << endl;
		if(find(ports.begin(), ports.end(), port)==ports.end()) { // Port is free
			cout << "Port is free" << endl;
			return port;
		} else { // Port is occupied
			cout << "Port is busy" << endl;
			continue;
		}
		return -1; // No ports free
	}
	return -1; // No ports exist
}

int Archiver::EraseID(unsigned long _id) {
    vector<unsigned long> _buffer;
    if(LoadIDs(_buffer)!=0) return -1;
	bool isErased = false;
    for(size_t i=0;i<_buffer.size();i++) {
  		if(_buffer[i]==_id) {
  			_buffer.erase(_buffer.begin()+i);
  			isErased = true;
  		}
    }
	if(isErased) {
		return writeIDVector(_buffer);
	}
    return -1;
}

int Archiver::PushID(unsigned long _id) {
    vector<unsigned long> _buffer;
    if(LoadIDs(_buffer)!=0) return -1;
    _buffer.push_back(_id);
    return writeIDVector(_buffer);
}

int Archiver::LoadIDs(vector<unsigned long> &_ids) {
    ifstream idStream;
    string buffer;

    idStream.open(ARCHIVECONFIGPATH);
    if(idStream.is_open()) {
    	while(getline(idStream,buffer,'[')) {
    	  if(buffer=="\n") continue; // At the end of file (?)
    	  _ids.push_back(stoul(buffer));
	}
	idStream.close();
	return 0;
    } else return -1;
}

int Archiver::LoadArchive(unsigned long _id, Archive &_archive) {

    _archive.ID = _id;

    string path = getPath(_id);
    ifstream archiveStream;

    string buffer;
    vector<string> rawData;

    archiveStream.open(path);
    if(archiveStream.is_open()) {
		while(getline(archiveStream,buffer,'[')) {
			rawData.push_back(buffer);
		}
		archiveStream.close();

		for(string i : rawData) {
			switch(i[0]-'0') {
				case 0: {
					_archive.Name = i.substr(1);
					break;
				}
				case 1: {
					_archive.LaunchPath = i.substr(1);
					break;
				}
				case 2: {
					_archive.Ram = stoi(i.substr(1));
					break;
				}
				case 3: {
					_archive.AccessIDs.push_back(stoul(i.substr(1)));
					break;
				}
				case 4: {
					_archive.Port = stoi(i.substr(1));
					break;
				}
				case 5: {
					_archive.Java = stoi(i.substr(1));
					break;
				}
				case 6: {
					_archive.NOA_AutoReboot = (bool)stoi(i.substr(1));
					break;
				}
				case 7: {
					_archive.NOA_LogReading = (bool)stoi(i.substr(1));
					break;
				}
				default: {
					cout << "Error: Invalid data detected in file" << endl;
					return -1;
				}
			}
		}
		return 0;
    } else
	{
		cout << "Fail to open file; errno=" << errno << endl;
		return -1;
	}
}

int Archiver::SaveArchive(unsigned long _id, Archive &_archive) {
    // Generate path
    string path = getPath(_id);
    ofstream archiveStream;

    archiveStream.open(path);
    if(archiveStream.is_open()) {
	archiveStream << "0" << _archive.Name << '[';
	archiveStream << "1" << _archive.LaunchPath << '[';
	archiveStream << "2" << _archive.Ram << '[';
	for(auto id : _archive.AccessIDs) {
	    archiveStream << "3" << id << '[';
	}
	archiveStream << "4" << _archive.Port << '[';
	archiveStream << "5" << _archive.Java << '[';
	archiveStream << "6" << (int)_archive.NOA_AutoReboot << '[';
	archiveStream << "7" << (int)_archive.NOA_LogReading << '[';

	archiveStream.close();
	return 0;

    } else return -1;
}

int Archiver::GetNewHash(unsigned long& _buffer) {
    hash<string> hash_obj;
    sleep(1);
    _buffer = hash_obj(to_string(static_cast<long>(time(nullptr)))); // Generate hash from epoch
    return 0;
}
