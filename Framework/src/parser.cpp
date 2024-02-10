// Header
#include "parser.hpp"
#include "sysutils.hpp"
#include "authenticator.hpp"
#include "netcom.hpp" // For constructor / NETCOM functions

// Defines
#define ENDOFTRANSMIT "\\\\_!end_!\\\\"
#define SLOGCUTOFF 20000
#define PARSELOOPSLP 50 // In ms
#define DELIMITER '|'
#define MAXFAILS 20

// Types
#include <string>
#include <vector>
#include <thread>
#include <memory>
#include <limits>

// Streams
#include <sstream>
#include <fstream>
#include <iostream>

// Functions
#include <unistd.h>
#include <algorithm>

// Namespaces
using namespace std;

ActiveParser::ActiveParser(NetworkCommunicator* netcom, map<unsigned long, shared_ptr<ServerModule>>* serverModules) {
    netcomRef = netcom;
    serverModuleRefs = serverModules;

    // Init parsehelper
    parsehelper["getval"] = GETVAL; // -> Call
    parsehelper["setval"] = SETVAL;
    parsehelper["dynamicmod"] = DYNAMICMOD;
    parsehelper["elevatereq"] = ELEVATEREQ;
    parsehelper["pwr"] = PWR; // -> Primary Argument
    parsehelper["log"] = LOG;
    parsehelper["cpu"] = CPU;
    parsehelper["mem"] = MEM;
    parsehelper["sws"] = SWS;
    parsehelper["cred"] = CRED;
    parsehelper["mod"] = MOD;
    parsehelper["add"] = ADD;
    parsehelper["del"] = DEL;
    parsehelper["get"] = GET;
    parsehelper["portprep"] = PORTPREP;
    parsehelper["overwrite"] = OVERWRITE;
    parsehelper["authorise"] = AUTHORISE;
    parsehelper["frameworkshutdown"] = FRAMEWORKSHUTDOWN; // -> (Framework power arguments)
    parsehelper["frameworkrestart"] = FRAMEWORKRESTART;
    parsehelper["frameworkpowerdown"] = FRAMEWORKPOWERDOWN;
    parsehelper["frameworkreboot"] = FRAMEWORKREBOOT;
    parsehelper["frameworkkillall"] = KILLALL;

    sysutil.StartAsyncCPUSys(SystemCPU);
    sysutil.StartAsyncMemSys(SystemMEM);

}

ActiveParser::~ActiveParser() {
    sysutil.StopSync();
}

vector<string> ActiveParser::tokenize(string in, char delim) {
    stringstream sstream(in);
    vector<string> out;
    string token;
    while(getline(sstream, token, delim)) {
        out.push_back(token);
    }
    return out;
}

int ActiveParser::getParseHelperVal(const string in) {
    auto parsehelperit = parsehelper.find(in);
    if(parsehelperit==parsehelper.end()) return -1;
    return parsehelperit->second;
}

int ActiveParser::executeDemand(const Demand demand, shared_ptr<ServerModule> serverModule, Connection* connection, map<unsigned long, shared_ptr<ServerModule>>& globalModules) {
    switch(demand.Command) {
        case SETVAL: {
            switch(demand.PrimaryArgument) {
                case PWR: {
                    if(serverModule==nullptr) {
                        stringstream errorstream;
                        errorstream << to_string(-1) << DELIMITER << "No Module Assigned";
                        netcomRef->WriteIncomingConnection(connection->sockfd, errorstream.str());
                        break;
                    }
                    try {
                        int pwrMode = stoi(demand.DataArgument);
                        if(pwrMode==START) netcomRef->WriteIncomingConnection(connection->sockfd, to_string(serverModule->Start()));
                        if(pwrMode==STOP) netcomRef->WriteIncomingConnection(connection->sockfd, to_string(serverModule->Stop()));
                        if(pwrMode==RESTART) netcomRef->WriteIncomingConnection(connection->sockfd, to_string(serverModule->Restart()));
                        if(pwrMode==KILL) netcomRef->WriteIncomingConnection(connection->sockfd, to_string(serverModule->Kill()));
                        break;
                    } catch (...) {
                        stringstream errorstream;
                        errorstream << to_string(-1) << DELIMITER << "Invalid Power Action";
                        netcomRef->WriteIncomingConnection(connection->sockfd, errorstream.str());
                        break;
                    }
                }
                case LOG: {
                    if(serverModule==nullptr) {
                        stringstream errorstream;
                        errorstream << to_string(-1) << DELIMITER << "No Module Assigned";
                        netcomRef->WriteIncomingConnection(connection->sockfd, errorstream.str());
                        break;
                    }
                    netcomRef->WriteIncomingConnection(connection->sockfd, to_string(serverModule->SendCommand(demand.DataArgument)));
                    break;
                }
                case MOD: {
                    unsigned long moduleid;
                    try {
                        moduleid = stoul(demand.DataArgument);
                    } catch (...) {
                        stringstream errorstream;
                        errorstream << to_string(-1) << DELIMITER << "Invalid Module ID Format";
                        netcomRef->WriteIncomingConnection(connection->sockfd, errorstream.str());
                        break;
                    }
                    auto globalIterator = globalModules.find(moduleid);
                    if(globalIterator == globalModules.end()) { // Invalid module ID
                        stringstream errorstream;
                        errorstream << to_string(-1) << DELIMITER << "Module ID Not Found";
                        netcomRef->WriteIncomingConnection(connection->sockfd, errorstream.str());
                        break;
                    }
                    assignedModules[connection->sockfd] = moduleid;
                    netcomRef->WriteIncomingConnection(connection->sockfd, to_string(0));
                    break;
                }
                default: { // Invalid API request
                    stringstream errorstream;
                    errorstream << to_string(-1) << DELIMITER << "Invalid Request";
                    netcomRef->WriteIncomingConnection(connection->sockfd, errorstream.str());
                    break;
                }
            }
            break;
        }

        case GETVAL: {
            switch(demand.PrimaryArgument) {
                case PWR: {
                    if(serverModule==nullptr) {
                        stringstream errorstream;
                        errorstream << to_string(-1) << DELIMITER << "No Module Assigned";
                        netcomRef->WriteIncomingConnection(connection->sockfd, errorstream.str());
                        break;
                    }
                    netcomRef->WriteIncomingConnection(connection->sockfd, to_string((int)serverModule->State()));
                    break;
                }
                case LOG: {
                    if(serverModule==nullptr) {
                        stringstream errorstream;
                        errorstream << to_string(-1) << DELIMITER << "No Module Assigned";
                        netcomRef->WriteIncomingConnection(connection->sockfd, errorstream.str());
                        break;
                    }
                    string logBuf;
                    serverModule->GetLog(logBuf);
                    netcomRef->WriteIncomingConnection(connection->sockfd, logBuf);
                    break;
                }
                case SLOG: {
                    if(serverModule==nullptr) {
                        stringstream errorstream;
                        errorstream << to_string(-1) << DELIMITER << "No Module Assigned";
                        netcomRef->WriteIncomingConnection(connection->sockfd, errorstream.str());
                        break;
                    }
                    string logBuf;
                    serverModule->GetLog(logBuf);
                    netcomRef->WriteIncomingConnection(connection->sockfd, logBuf.substr(logBuf.size()-SLOGCUTOFF));
                    break;
                }
                case CPU: {
                    if(serverModule==nullptr) {
                        stringstream errorstream;
                        errorstream << to_string(-1) << DELIMITER << "No Module Assigned";
                        netcomRef->WriteIncomingConnection(connection->sockfd, errorstream.str());
                        break;
                    }
                    stringstream buffer;
                    buffer << SysUtils::GetSystemCores() << DELIMITER << SystemCPU << DELIMITER << serverModule->CpuUtilization;
                    netcomRef->WriteIncomingConnection(connection->sockfd, buffer.str());
                    break;
                }
                case MEM: {
                    if(serverModule==nullptr) {
                        stringstream errorstream;
                        errorstream << to_string(-1) << DELIMITER << "No Module Assigned";
                        netcomRef->WriteIncomingConnection(connection->sockfd, errorstream.str());
                        break;
                    }
                    stringstream buffer;
                    buffer << SysUtils::GetSystemMemory() << DELIMITER << SystemMEM << DELIMITER << serverModule->MemoryVmSize << DELIMITER << serverModule->MemoryVmRSS;
                    netcomRef->WriteIncomingConnection(connection->sockfd, buffer.str());
                    break;
                }
                case SWS: {
                    if(serverModule==nullptr) {
                        stringstream errorstream;
                        errorstream << to_string(-1) << DELIMITER << "No Module Assigned";
                        netcomRef->WriteIncomingConnection(connection->sockfd, errorstream.str());
                        break;
                    }
                    stringstream buffer;
                    buffer << (int)serverModule->State() << DELIMITER << DELIMITER;
                    buffer << SysUtils::GetSystemCores() << DELIMITER << SystemCPU << DELIMITER << serverModule->CpuUtilization << DELIMITER << DELIMITER;
                    buffer << SysUtils::GetSystemMemory() << DELIMITER << SystemMEM << DELIMITER << serverModule->MemoryVmSize << DELIMITER << serverModule->MemoryVmRSS << DELIMITER << DELIMITER;
                    string log;
                    serverModule->GetLog(log);
                    buffer << log << DELIMITER << DELIMITER;
                    netcomRef->WriteIncomingConnection(connection->sockfd, buffer.str());
                    break;
                }
                case CRED: {
                    stringstream buffer;
                    vector<string> creds = Authenticator::GetPublicCredentials();
                    for(const auto &i : creds) {
                        buffer << i << DELIMITER;
                    }
                    netcomRef->WriteIncomingConnection(connection->sockfd, buffer.str());
                    break;
                }
                case MOD: {
                    string buf;
                    for(auto &i : *serverModuleRefs) {
                        buf = buf + to_string(i.second->ID) + DELIMITER + i.second->Name + DELIMITER + to_string(i.second->State()) + DELIMITER + DELIMITER;
                    }
                    netcomRef->WriteIncomingConnection(connection->sockfd, buf);
                    break;
                }
                default: { // Invalid API request
                    stringstream errorstream;
                    errorstream << to_string(-1) << DELIMITER << "Invalid Request";
                    netcomRef->WriteIncomingConnection(connection->sockfd, errorstream.str());
                    break;
                }
            }
            break;
        }

        case DYNAMICMOD: {
            switch(demand.PrimaryArgument) {
                case ADD: {
                    Archive archive;
                    int ram = 0;
                    int java = 0;
                    vector<string> tokens = tokenize(demand.DataArgument, DELIMITER);
                    if(tokens.size()!=4) {
                        stringstream errorstream;
                        errorstream << to_string(-1) << DELIMITER << "Invalid Token Count";
                        netcomRef->WriteIncomingConnection(connection->sockfd, errorstream.str());
                        break;
                    }
                    try {
                        ram = stoi(tokens[1]);
                        java = stoi(tokens[3]);
                    } catch (...) {
                        stringstream errorstream;
                        errorstream << to_string(-1) << DELIMITER << "Invalid Allocated RAM or JAVA version";
                        netcomRef->WriteIncomingConnection(connection->sockfd, errorstream.str());
                        break;
                    }
                    archive.Name = tokens[0];
                    archive.Ram = ram;
                    archive.LaunchPath = tokens[2];
                    unsigned long id;
                    Archiver::GetNewHash(id);
                    archive.ID = id;
                    archive.Java = java;
                    archive.Port = Archiver::GetNextPort();
                    if(archive.Port==-1) {
                        stringstream errorstream;
                        errorstream << to_string(-1) << DELIMITER << "No Free Port";
                        netcomRef->WriteIncomingConnection(connection->sockfd, errorstream.str());
                        break;
                    }
                    int returnval = Archiver::SaveArchive(id, archive);
                    if(returnval!=0) {
                        stringstream errorstream;
                        errorstream << to_string(-1) << DELIMITER << "Failed To Save Archive";
                        netcomRef->WriteIncomingConnection(connection->sockfd, errorstream.str());
                        break;
                    }
                    returnval = Archiver::PushID(id);
                    if(returnval!=0) {
                        stringstream errorstream;
                        errorstream << to_string(-1) << DELIMITER << "Failed To Push Archive ID";
                        netcomRef->WriteIncomingConnection(connection->sockfd, errorstream.str());
                        break;
                    }
                    globalModules[id] = shared_ptr<ServerModule>(new ServerModule(archive));
                    system(((string)"mkdir ./data/serverdata/"+to_string(archive.ID)).c_str());
                    netcomRef->WriteIncomingConnection(connection->sockfd, to_string(id));
                    break;
                }
                case PORTPREP: {
                    if(serverModule==nullptr) {
                        stringstream errorstream;
                        errorstream << to_string(-1) << DELIMITER << "No Module Assigned";
                        netcomRef->WriteIncomingConnection(connection->sockfd, errorstream.str());
                        break;
                    }
                    string propertieslocation = (string)"./data/serverdata/" + to_string(serverModule->ID) + (string)"/server.properties";
                    ifstream propstream(propertieslocation);
                    if(propstream.good()) { // Properties file exists!
                        stringstream buffer;
                        buffer << propstream.rdbuf();
                        vector<string> settings = tokenize(buffer.str(), '\n');
                        propstream.close();
                        bool mod = false;
                        for(auto &i : settings) {
                            if(i=="#Modified by Swirve Framework [v1.1]") i.erase();
                            if(i.find("server-port=")==string::npos) continue; // Skip if doesn't contain "server-port="
                            i = (string)"server-port=" + to_string(serverModule->AssignedPort); // Modify value in vector to the correct port value
                            mod = true;
                        }
                        if(!mod) { // Properties file didn't include a specified server port
                            ofstream newpropstream(propertieslocation, ios::app);
                            if(!newpropstream.good()) {
                                stringstream errorstream;
                                errorstream << to_string(-1) << DELIMITER << "Failed To Open Existing Config File";
                                netcomRef->WriteIncomingConnection(connection->sockfd, errorstream.str());
                                break;
                            }
                            newpropstream << "\n#Modified by Swirve Framework [v1.1]\n";
                            newpropstream << (string)"server-port=" + to_string(serverModule->AssignedPort) + "\n"; // Append port config to end of settings file
                            newpropstream.close();
                            netcomRef->WriteIncomingConnection(connection->sockfd, to_string(0));
                            break;
                        } else { // Properties file DID include a specified server port, but was overwritten by previous function in vector
                            ofstream newpropstream(propertieslocation);
                            if(!newpropstream.good()) {
                                stringstream errorstream;
                                errorstream << to_string(-1) << DELIMITER << "Failed To Overwrite Config File";
                                netcomRef->WriteIncomingConnection(connection->sockfd, errorstream.str());
                                break;
                            }
                            newpropstream << "#Modified by Swirve Framework [v1.1]\n";
                            for(const auto &i : settings) {
                                newpropstream << i << "\n";
                            }
                            newpropstream.close();
                            netcomRef->WriteIncomingConnection(connection->sockfd, to_string(0));
                            break;
                        }

                    } else { // Properties file does not exist
                        propstream.close();
                        stringstream buffer;
                        buffer << "server-port=" << serverModule->AssignedPort;
                        ofstream newpropstream(propertieslocation);
                        if(!newpropstream.good()) { // Failed to generate
                            stringstream errorstream;
                            errorstream << to_string(-1) << DELIMITER << "Failed To Create New Config File";
                            netcomRef->WriteIncomingConnection(connection->sockfd, errorstream.str());
                            break;
                        }
                        newpropstream << "#Created by Swirve Framework [v1.1]\n";
                        newpropstream << buffer.str();
                        newpropstream.close();
                        netcomRef->WriteIncomingConnection(connection->sockfd, to_string(0));
                        break;
                    }
                    break;
                }

                case DEL: {
                    unsigned long id;
                    try {
                        id = stoul(demand.DataArgument);
                    } catch (...) {
                        stringstream errorstream;
                        errorstream << to_string(-1) << DELIMITER << "Invalid Module ID";
                        netcomRef->WriteIncomingConnection(connection->sockfd, errorstream.str());
                        break;
                    }
                    int returnval = Archiver::EraseID(id);
                    if(returnval!=0) {
                        stringstream errorstream;
                        errorstream << to_string(-1) << DELIMITER << "Module ID Not Found";
                        netcomRef->WriteIncomingConnection(connection->sockfd, errorstream.str());
                    }
                    globalModules.erase(id);
                    netcomRef->WriteIncomingConnection(connection->sockfd, to_string(0));
                    break;
                }
                case GET: {
                    try {
                        stringstream buffer;
                        Archive archive;
                        int archive_ret = Archiver::LoadArchive(stoul(demand.DataArgument), archive);
                        if(archive_ret<0) {
                            stringstream errorstream;
                            errorstream << to_string(-1) << DELIMITER << "Module ID Not Found";
                            netcomRef->WriteIncomingConnection(connection->sockfd, errorstream.str());
                            break;
                        }
                        buffer << archive.Name << DELIMITER << archive.Ram << DELIMITER << archive.Port << DELIMITER << archive.LaunchPath << DELIMITER << archive.Java;
                        netcomRef->WriteIncomingConnection(connection->sockfd, buffer.str());
                        break;
                    } catch (...) {}
                    // Error
                    stringstream errorstream;
                    errorstream << to_string(-1) << DELIMITER << "Invalid Module ID";
                    netcomRef->WriteIncomingConnection(connection->sockfd, errorstream.str());
                    break;
                }
                case OVERWRITE: {
                  
                    Archive archive;
                    int ram = 0;
                    int java = 0;
                    unsigned long id = 0;
                    vector<string> tokens = tokenize(demand.DataArgument, DELIMITER);
                    if(tokens.size()!=5) {
                        stringstream errorstream;
                        errorstream << to_string(-1) << DELIMITER << "Invalid Token Count";
                        netcomRef->WriteIncomingConnection(connection->sockfd, errorstream.str());
                        break;
                    }
                    try {
                        id = stoul(tokens[0]);
                        ram = stoi(tokens[2]);
                        java = stoi(tokens[4]);
                    } catch (...) {
                        stringstream errorstream;
                        errorstream << to_string(-1) << DELIMITER << "Invalid ID, RAM or JAVA";
                        netcomRef->WriteIncomingConnection(connection->sockfd, errorstream.str());
                        break;
                    }

                    Archive oldArchive;
                    if(Archiver::LoadArchive(id, oldArchive)!=0) {
                        stringstream errorstream;
                        errorstream << to_string(-1) << DELIMITER << "Archive Could Not Load";
                        netcomRef->WriteIncomingConnection(connection->sockfd, errorstream.str());
                        break;
                    }

                    archive.ID = id;
                    archive.Name = tokens[1];
                    archive.Ram = ram;
                    archive.LaunchPath = tokens[3];
                    archive.Java = java;
                    archive.Port = oldArchive.Port;

                    auto globalIterator = globalModules.find(id);
                    if(globalIterator==globalModules.end()) { // Module doesn't exist
                        stringstream errorstream;
                        errorstream << to_string(-1) << DELIMITER << "Module ID Not Found";
                        netcomRef->WriteIncomingConnection(connection->sockfd, errorstream.str());
                        break;
                    }

                    int returnval = Archiver::SaveArchive(id, archive);
                    if(returnval!=0) {
                        stringstream errorstream;
                        errorstream << to_string(-1) << DELIMITER << "Failed To Save Archive";
                        netcomRef->WriteIncomingConnection(connection->sockfd, errorstream.str());
                        break;
                    }

                    globalIterator->second->Name = archive.Name;
                    globalIterator->second->LaunchPath = archive.LaunchPath;
                    globalIterator->second->RamAllocated = archive.Ram;
                    globalIterator->second->JavaVersion = archive.Java;

                    l.warn("Parser","Updating module data!!");
                    globalIterator->second->Configure(archive);


                    netcomRef->WriteIncomingConnection(connection->sockfd, to_string(0));

                    break;
                }
                default: {
                    stringstream errorstream;
                    errorstream << to_string(-1) << DELIMITER << "Invalid Request";
                    netcomRef->WriteIncomingConnection(connection->sockfd, errorstream.str());
                    break;
                }
            }
            break;
        }

        case ELEVATEREQ: {
            switch(demand.PrimaryArgument) {
                case FRAMEWORKSHUTDOWN: {
                    if(!authorisedClients[connection->sockfd]) { // Client isn't authorised
                        stringstream errorstream;
                        errorstream << to_string(-1) << DELIMITER << "Not Authorized";
                        netcomRef->WriteIncomingConnection(connection->sockfd, errorstream.str());
                        break;
                    }
                    netcomRef->WriteIncomingConnection(connection->sockfd, to_string(0));
                    broadcastMessage("Server will be powering down soon.");
                    sleep(5);
                    broadcastMessage("Powering down...");
                    broadcastCommand("stop");
                    sleep(10);
                    broadcastMessage("Framework shutting down");
                    netcomRef->WriteIncomingConnection(connection->sockfd, ENDOFTRANSMIT); // As connection will be killed before Parser and Netcom gets a chance to write ENDOFTRANSMIT at bottom of execution
                    system("sudo systemctl stop swirve-framework.service"); // TODO: Replace with actual name ALL BELOW!!
                    break;
                }

                case FRAMEWORKRESTART: {
                    if(!authorisedClients[connection->sockfd]) { // Client isn't authorised
                        stringstream errorstream;
                        errorstream << to_string(-1) << DELIMITER << "Not Authorized";
                        netcomRef->WriteIncomingConnection(connection->sockfd, errorstream.str());
                        break;
                    }
                    netcomRef->WriteIncomingConnection(connection->sockfd, to_string(0));
                    broadcastMessage("Server will be powering down soon.");
                    sleep(5);
                    broadcastMessage("Powering down...");
                    broadcastCommand("stop");
                    sleep(10);
                    broadcastMessage("Framework restarting");
                    netcomRef->WriteIncomingConnection(connection->sockfd, ENDOFTRANSMIT); // As connection will be killed before Parser and Netcom gets a chance to write ENDOFTRANSMIT at bottom of execution
                    system("sudo systemctl restart swirve-framework.service");
                    break;
                }

                case FRAMEWORKPOWERDOWN: {
                    if(!authorisedClients[connection->sockfd]) { // Client isn't authorised
                        stringstream errorstream;
                        errorstream << to_string(-1) << DELIMITER << "Not Authorized";
                        netcomRef->WriteIncomingConnection(connection->sockfd, errorstream.str());
                        break;
                    }
                    netcomRef->WriteIncomingConnection(connection->sockfd, to_string(0));
                    broadcastMessage("Server will be powering down soon.");
                    sleep(5);
                    broadcastMessage("Powering down...");
                    broadcastCommand("stop");
                    sleep(10);
                    broadcastMessage("System shutting down");
                    netcomRef->WriteIncomingConnection(connection->sockfd, ENDOFTRANSMIT); // As connection will be killed before Parser and Netcom gets a chance to write ENDOFTRANSMIT at bottom of execution
                    system("sudo systemctl shutdown");
                    break;
                }

                case FRAMEWORKREBOOT: {
                    if(!authorisedClients[connection->sockfd]) { // Client isn't authorised
                        stringstream errorstream;
                        errorstream << to_string(-1) << DELIMITER << "Not Authorized";
                        netcomRef->WriteIncomingConnection(connection->sockfd, errorstream.str());
                        break;
                    }
                    netcomRef->WriteIncomingConnection(connection->sockfd, to_string(0));
                    broadcastMessage("Server will be powering down soon.");
                    sleep(5);
                    broadcastMessage("Powering down...");
                    broadcastCommand("stop");
                    sleep(10);
                    broadcastMessage("System rebooting");
                    netcomRef->WriteIncomingConnection(connection->sockfd, ENDOFTRANSMIT); // As connection will be killed before Parser and Netcom gets a chance to write ENDOFTRANSMIT at bottom of execution
                    system("sudo systemctl reboot");
                    break;
                }

                case KILLALL: {
                    if(!authorisedClients[connection->sockfd]) { // Client isn't authorised
                        stringstream errorstream;
                        errorstream << to_string(-1) << DELIMITER << "Not Authorized";
                        netcomRef->WriteIncomingConnection(connection->sockfd, errorstream.str());
                        break;
                    }
                    netcomRef->WriteIncomingConnection(connection->sockfd, to_string(0));
                    netcomRef->WriteIncomingConnection(connection->sockfd, ENDOFTRANSMIT); // As connection will be killed before Parser and Netcom gets a chance to write ENDOFTRANSMIT at bottom of execution
                    netcomRef->KillConnections();
                    authorisedClients.clear();
                    break;
                }

                case AUTHORISE: {
                    string key = demand.DataArgument;
                    key.erase(remove(key.begin(), key.end(), '\r'), key.end());
                    bool authenticated = Authenticator::AuthenticateKey(key) == 0 ? true : false;
                    if(authenticated) {
                        authorisedClients[connection->sockfd] = true;
                        netcomRef->WriteIncomingConnection(connection->sockfd, to_string(0));
                    } else {
                        stringstream errorstream;
                        errorstream << to_string(-1) << DELIMITER << "Invalid Key";
                        netcomRef->WriteIncomingConnection(connection->sockfd, errorstream.str());
                    }
                    break;
                }
                default: { // Invalid API request
                    stringstream errorstream;
                    errorstream << to_string(-1) << DELIMITER << "Invalid Request";
                    netcomRef->WriteIncomingConnection(connection->sockfd, errorstream.str());
                    break;
                }
            }
            break;
        }

        default: { // Invalid API request
            stringstream errorstream;
            errorstream << to_string(-1) << DELIMITER << "Invalid Request";
            netcomRef->WriteIncomingConnection(connection->sockfd, errorstream.str());
            break;
        }
    } // TODO: ERROR HANDLING
	netcomRef->WriteIncomingConnection(connection->sockfd, ENDOFTRANSMIT);
    return 0;
}

int ActiveParser::parseDemand(const std::string buffer, Demand &demand) {

    // Fetch string equivalent of each field (dataCommand;primaryarg;dataarg)
    stringstream bufferSS(buffer);
    string token;
    getline(bufferSS,token,DELIMITER);
    string dCommand = token;
    getline(bufferSS,token,DELIMITER);
    string dPrimaryArgument = token;
    string dDataArgument;
    while(getline(bufferSS,token, '\r')) dDataArgument += token; // Append rest of request to the data argument (get next "cr", so before next carriage return, and API requests to Framework doesn't include such).
                                                                 // I chose carriage returns as if Framework is recieving data from TELNET on Darwin commands won't be malformed (hopefully)
    // Fetch native equivalent, often in terms of enumerables (=> int)
    int iCommand = getParseHelperVal(dCommand);
    int iPrimaryArgument = getParseHelperVal(dPrimaryArgument);
    demand.Command = iCommand;
    demand.PrimaryArgument = iPrimaryArgument;
    demand.DataArgument = dDataArgument;
    return 0;
}

void ActiveParser::parseLoop(bool* running, NetworkCommunicator* netcom, map<unsigned long, shared_ptr<ServerModule>>* serverModules) {
    int netcomFails = 0;
    vector<Connection> connections;
    while(*running) {
        if(netcomFails>MAXFAILS) {
            l.fatal("Parser","Internal Framework Exception: [NRUN_NFAI_GREA_FAIL]");
            l.fatal("Parser","Terminating ActiveParser...");
            *running = false;
            break;
        }
		usleep(1000*PARSELOOPSLP); // Sleep PARSELOOPSLPms (ms * 1000 => us)
		if(netcom->ReadIncomingConnections(connections, &authorisedClients)<0){
            l.warn("Parser","Failed to scan incoming connections for data, Framework reboot advised.");
            l.warn("Parser","Incrementing fail counter!");
            netcomFails++;
            continue;
        }
		for(Connection &connection : connections) {
			if(connection.buffer[0]==0) { // Connection "empty", aka nothing is new
                continue;
            }
			Demand demand;
			if(parseDemand(connection.buffer, demand)<0) continue;
            unsigned long assignedModule = assignedModules[connection.sockfd]; // Fetch the module ID assigned to TCP socket filedescriptor
			auto moduleIterator = serverModules->find(assignedModule); // Fetch the actual module in memory
            if(moduleIterator==serverModules->end()) { // The connection hasn't assigned a module (legacy, Swirve Userclient v1.0)
                l.warn("Parser", (string)"Request issued without assigned ID: TCPFD " + to_string(connection.sockfd));
                vector<unsigned long> ids;             // Down; Create a module
                int returnval = Archiver::LoadIDs(ids);
                if(returnval!=0 || ids.size()<=0) { // Failed to find an ID, so executes demand with nullpointer as module
                    executeDemand(demand, nullptr, &connection, *serverModules);
                    continue; // Skip rest of loop so that it won't execute demand twice (and segfault due to missing module, moduleIterator->second generates segfault if not found)
                }
                assignedModules[connection.sockfd] = ids[0];
                assignedModule = ids[0];
                moduleIterator = serverModules->find(assignedModule);
                l.info("Parser", (string)"Generated id: " + to_string(moduleIterator->first));
            }

            executeDemand(demand, moduleIterator->second, &connection, *serverModules);
		}
    }
    l.warn("Parser","Offline");
}

int ActiveParser::StartParser() {
    shouldRunParser = true;
    runParser = true;
    parserDaemon = thread(&ActiveParser::parseLoop,this,&runParser,netcomRef,serverModuleRefs);
    return 0;
}

int ActiveParser::ParserException() {
    return shouldRunParser == true && runParser == false ? -1 : 0;
}

int ActiveParser::StopParser() {
    shouldRunParser = false;
    runParser = false;
    parserDaemon.join();
    sysutil.StopSync();
    return 0;
}

void ActiveParser::broadcastMessage(string message) {
    for(auto &sModule : *serverModuleRefs) {
        sModule.second->SendCommand("tellraw @a [{\"color\":\"aqua\",\"text\":\"[ SWIRVE ] \"}, {\"color\":\"white\",\"text\":\"" + message + "\"}]");
    }
}

void ActiveParser::broadcastCommand(string command) {
    for(auto &sModule : *serverModuleRefs) {
        l.warn("Parser", "Powering down server " + sModule.second->Name);
        sModule.second->SendCommand(command);
    }
}
