// Header
#include "../parser/parser.h"
#include "../netcom/netcom.h" // For constructor / NETCOM functions

// Defines
#define ENDOFTRANSMIT "\\\\_!end_!\\\\"

// Types
#include <string>
#include <vector>
#include <thread>

// Streams
#include <sstream>
#include <iostream>

// Functions
#include <unistd.h>


// Namespaces
using namespace std;

ActiveParser::ActiveParser(NetworkCommunicator* netcom, ServerModule* serverModule) {
    netcomRef = netcom;
    serverModuleRef = serverModule;

    // Init parsehelper
    parsehelper["getval"] = GETVAL;
    parsehelper["setval"] = SETVAL;
    parsehelper["pwr"] = PWR;
    parsehelper["log"] = LOG;
}

int ActiveParser::getParseHelperVal(const string in) {
    auto parsehelperit = parsehelper.find(in);
    if(parsehelperit==parsehelper.end()) return -1;
    return parsehelperit->second;
}

int ActiveParser::executeDemand(const Demand demand, ServerModule* serverModule, Connection* connection) {
    switch(demand.Command) {
	case SETVAL:
	    switch(demand.PrimaryArgument) {
			case PWR: {
				int pwrMode = stoi(demand.DataArgument);
				if(pwrMode==START) netcomRef->WriteIncomingConnection(connection->sockfd, to_string(serverModule->Start()));
				if(pwrMode==STOP) netcomRef->WriteIncomingConnection(connection->sockfd, to_string(serverModule->Stop()));
				if(pwrMode==RESTART) netcomRef->WriteIncomingConnection(connection->sockfd, to_string(serverModule->Restart()));
				if(pwrMode==KILL) netcomRef->WriteIncomingConnection(connection->sockfd, to_string(serverModule->Kill()));
				break;
			}
			case LOG: {
				netcomRef->WriteIncomingConnection(connection->sockfd, to_string(serverModule->SendCommand(demand.DataArgument)));
				break;
			}
	    }
	    break;

	case GETVAL:
	    switch(demand.PrimaryArgument) {
	    	case PWR:
		    netcomRef->WriteIncomingConnection(connection->sockfd, to_string((int)serverModule->State()));
		    break;
		case LOG:
		    string logBuf;
		    serverModule->GetLog(logBuf);
		    netcomRef->WriteIncomingConnection(connection->sockfd, logBuf);
		    break;
	    }
	    break;
    } // TODO: ERROR HANDLING
	netcomRef->WriteIncomingConnection(connection->sockfd, ENDOFTRANSMIT);
    return 0;
}

int ActiveParser::parseDemand(const std::string buffer, Demand &demand) {
    stringstream bufferSS(buffer);
    string token;
    if(!getline(bufferSS,token,DELIMITER)) return -1;
    string dCommand = token;
    if(!getline(bufferSS,token,DELIMITER)) return -1;
    string dPrimaryArgument = token;
    if(!getline(bufferSS,token,DELIMITER)) return -1;
    string dDataArgument = token;

    // Process values
    int iCommand = getParseHelperVal(dCommand);
    int iPrimaryArgument = getParseHelperVal(dPrimaryArgument);
    if(iCommand!=-1) demand.Command = iCommand;
    if(iPrimaryArgument!=-1) demand.PrimaryArgument = iPrimaryArgument;
    demand.DataArgument = dDataArgument;


    // if(!getline(bufferSS,token,DELIMITER)) return -1; // TODO: IMPLEMENT MULTI SERVER SUPPORT
    // demand.Target = token;
    return 0;
}

void ActiveParser::parseLoop(bool* running, NetworkCommunicator* netcom, ServerModule* serverModule) {
    vector<Connection> connections;
    while(*running) {
		usleep(1000*50); // Sleep 50ms (1000uS=1ms => *50)
		if(netcom->ReadIncomingConnections(connections)<0) continue;
		for(Connection &connection : connections) {
			if(connection.buffer[0]==0) continue;
			Demand demand;
			if(parseDemand(connection.buffer, demand)<0) continue;
			executeDemand(demand, serverModule, &connection);
		}
    }
}

int ActiveParser::StartParser() {
    runParser = true;
    parserDaemon = thread(&ActiveParser::parseLoop,this,&runParser,netcomRef,serverModuleRef);
    return 0;
}

int ActiveParser::StopParser() {
    runParser = false;
    parserDaemon.join();
    return 0;
}
