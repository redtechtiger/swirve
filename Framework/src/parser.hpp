#ifndef PARSER_H
#define PARSER_H

// Header file
#include "netcom.hpp"
#include "module.hpp"
#include "log.hpp"

// Types
#include <string>
#include <vector>
#include <thread>
#include <memory>
#include <map>

// Enums
enum ECommand {SETVAL, GETVAL, DYNAMICMOD, ELEVATEREQ};
enum EArgument {PWR, LOG, SLOG, CPU, MEM, SWS, CRED, MOD, ADD, DEL, GET, PORTPREP, OVERWRITE, AUTHORISE};
// enum EPowerAction {START, STOP, KILL, RESTART}; -- These are not intended to be uncommented! Simply notes to remember the ENUMs
// enum EPowerState {OFFLINE, STARTING, ONLINE, STOPPING, RESTARTING, KILLING, FAULT}; -- -||-
enum EFrameworkPowerAction {FRAMEWORKSHUTDOWN, FRAMEWORKRESTART, FRAMEWORKPOWERDOWN, FRAMEWORKREBOOT, KILLALL};

// Structs
struct Demand {
    int Command;
    int PrimaryArgument;
    std::string DataArgument;
};

class ActiveParser {

	public:

		ActiveParser(NetworkCommunicator* netcom, std::map<unsigned long, std::shared_ptr<ServerModule>>* serverModules);
		~ActiveParser();

    private:

    	NetworkCommunicator* netcomRef;
		std::map<unsigned long, std::shared_ptr<ServerModule>>* serverModuleRefs;
		std::map<int,unsigned long> assignedModules;
		std::map<int,bool> authorisedClients;

		std::vector<std::string> tokenize(std::string in, char delim);
	
		bool runParser = false;
		bool shouldRunParser = false;
		std::thread parserDaemon;
	
		int parseDemand(const std::string buffer, Demand &demand);
		int executeDemand(const Demand demand, std::shared_ptr<ServerModule> serverModule, Connection* connection, std::map<unsigned long, std::shared_ptr<ServerModule>>& globalModules);
		void parseLoop(bool* running, NetworkCommunicator* netcom, std::map<unsigned long, std::shared_ptr<ServerModule>>* serverModules);

		std::map<std::string,int> parsehelper;
		int getParseHelperVal(const std::string in);

		void broadcastMessage(std::string message);
		void broadcastCommand(std::string command);

		int SystemCPU = 0;
		int SystemMEM = 0;
		SysUtils sysutil;

		Logger l;

    public:
        
		int StartParser();
		int ParserException();
		int StopParser();
};

#endif
