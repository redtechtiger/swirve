#include <iostream>
#include "../modules/module.h"

ServerModule::ServerModule(Archive _archive) {
	Name = _archive.Name;
	ID = _archive.ID;
	LaunchPath = _archive.LaunchPath;
	for(auto i : _archive.AccessIDs)
		AccessIDs.push_back(i);
}
