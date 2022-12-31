#include <iostream>
#include "../modules/module.h"

ServerModule::ServerModule(Archive _archive) {
	std::cout << "INIT SERVERMODULE\n";
	Name = _archive.Name;
	LaunchPath = _archive.LaunchPath;
	for(auto i : _archive.AccessIDs)
		AccessIDs.push_back(i);
}
