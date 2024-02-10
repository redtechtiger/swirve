#include <iostream>
#include <string>
#include "module.hpp"
#include "unistd.h"
#include "limits.h"

using namespace std;

ServerModule::ServerModule(Archive _archive) {
	Name = _archive.Name;
	ID = _archive.ID;
	LaunchPath = _archive.LaunchPath;
	RamAllocated = _archive.Ram;
	AssignedPort = _archive.Port;
	for(auto i : _archive.AccessIDs)
		AccessIDs.push_back(i);

	JavaVersion = _archive.Java;
	AutoRebootOnOutage = _archive.NOA_AutoReboot;
	AutoPipeClearing = _archive.NOA_LogReading;

	char path[PATH_MAX];
	if(getcwd(path, sizeof(path))!=NULL) {
		string absoluteJarPath = (string)path + (string)RELATIVESERVERLOCATION + to_string(ID) + (string)"/" + LaunchPath;
		RealPath = absoluteJarPath;
		Config(RealPath, RamAllocated, JavaVersion);
	} else {
		Config(LaunchPath, RamAllocated, JavaVersion);
	}
}

void ServerModule::Configure(Archive _archive) {
	Name = _archive.Name;
	ID = _archive.ID;
	LaunchPath = _archive.LaunchPath;
	RamAllocated = _archive.Ram;
	AssignedPort = _archive.Port;
	for(auto i : _archive.AccessIDs)
		AccessIDs.push_back(i);

	JavaVersion = _archive.Java;
	AutoRebootOnOutage = _archive.NOA_AutoReboot;
	AutoPipeClearing = _archive.NOA_LogReading;

	char path[PATH_MAX];
	if(getcwd(path, sizeof(path))!=NULL) {
		string absoluteJarPath = (string)path + (string)RELATIVESERVERLOCATION + to_string(ID) + (string)"/" + LaunchPath;
		RealPath = absoluteJarPath;
		Config(RealPath, RamAllocated, JavaVersion);
	} else {
		Config(LaunchPath, RamAllocated, JavaVersion);
	}
}