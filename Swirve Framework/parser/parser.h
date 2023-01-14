#ifndef PARSER_H
#define PARSER_H

// Defines
#define DELIMITER '|'

// Header file
#include "../netcom/netcom.h"
#include "../modules/module.h"

// Types
#include <string>
#include <vector>
#include <thread>
#include <map>

// Enums
enum ECommand {SETVAL, GETVAL};
enum EArgument {PWR, LOG};
// enum EPowerAction {START, STOP, KILL, RESTART};
// enum EPowerState {OFFLINE, STARTING, ONLINE, STOPPING, RESTARTING, KILLING, FAULT};
// Structs
struct Demand {
    int Command;
    int PrimaryArgument;
    std::string DataArgument;
    // std::string Target; // TODO: IMPLEMENT MULTI SERVER SUPPORT
};

class ActiveParser {
    private:
    	NetworkCommunicator* netcomRef;
	ServerModule* serverModuleRef;	
	
	bool runParser = false;
	std::thread parserDaemon;
	
	int parseDemand(const std::string buffer, Demand &demand);
	int executeDemand(const Demand demand, ServerModule* sevrerModule, Connection* connection);
	void parseLoop(bool* running, NetworkCommunicator* netcom, ServerModule* serverModule);

	std::map<std::string,int> parsehelper;
	int getParseHelperVal(const std::string in);

    public:
        ActiveParser(NetworkCommunicator* netcom, ServerModule* serverModule);
	int StartParser();
	int StopParser();
};


#endif
