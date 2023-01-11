#include <iostream>
#include <unistd.h>
#include "../handler/handler.h"
#include "../modules/module.h"
#include "../archive/archive.h"
#include "../netcom/netcom.h"
#include "../logger/log.h"

void showLogo() {
    std::cout << "\u001b[26m===================================================================================================\n";
    std::cout << "\n";
    std::cout << "  _________       .__                     \n";
    std::cout << " /   _____/_  _  _|__|_________  __ ____  \n";
    std::cout << " \\_____  \\\\ \\/ \\/ /  \\_  __ \\  \\/ // __ \\ \n";
    std::cout << " /        \\\\     /|  ||  | \\/\\   /\\  ___/ \n";
    std::cout << "/_______  / \\/\\_/ |__||__|    \\_/  \\___  >\n";
    std::cout << "        \\/                             \\/\n";
    std::cout << "						Framework 1.0.0-ALPHA.1\n";
    std::cout << "===================================================================================================\n\u001b[0m";
    std::cout << "DISCLAIMER: THIS SOFTWARE IS WRITTEN BY AND BELONGS TO REDTECHTIGER MEDIA. USE IS PROHIBITED OUTSIDE THE LPQ SERVER COMMUNITY. IF SOURCE CODE IS LEAKED, PLEASE CONTACT THE OWNER IMMEDIATELY.\n";
}

void shell_loop(MinecraftHandler *handler) {
    std::string input;
    std::string output;
    while(1==1) {
	std::cout << ">";
	std::getline(std::cin,input);
	if(input=="help") {
	    std::cout << "[Shell]: Commands: help, log, status, stop, start, reboot, kill\n";
	} else if(input=="log") {
	    std::cout << "[Server Output]:\n";
	    handler->GetLog(output);
	    std::cout << output << "\n";
	} else if(input=="status") {
	    std::cout << "[Shell]: Server status [" << handler->State() << "]\n";
	} else if(input=="start") {
	    std::cout << "[Shell]: Server start " << (handler->Start() == 0 ? "OK" : "FAIL") << "\n";
	} else if(input=="stop") {
	    std::cout << "[Shell]: Server stop " << (handler->Stop() == 0 ? "OK" : "FAIL") << "\n";
	} else if(input=="reboot") {
	    std::cout << "[Shell]: Server reboot " << (handler->Restart() == 0 ? "OK" : "FAIL") << "\n";
	} else if(input=="kill") {
	    std::cout << "[Shell]: Server kill " << (handler->Kill() == 0 ? "OK" : "FAIL") << "\n";
	} else if(input=="exit") {
	    std::cout << "[Shell]: Bye\n";
	    break;
	} else {
	    std::cout << "[Shell]: Treating command as server input\n";
	    handler->SendCommand(input);
	}
	fflush(stdout);
    }
}

template <typename T>
void print_vector(T couts) {
    for(const auto &i : couts) {
	std::cout << i << "\n";
    }
}

void core_entry() {
    std::cout << "Core-preinit: Initiating framework core...\n";
    Logger l;
    showLogo();

    std::cout << "Core: Starting services...\n"; 
    NetworkCommunicator netcom;
    std::vector<ServerModule> modules;

    l.logLengthyFunction("Core: Loading server modules from archives...");
    std::vector<ulong> ids; 
    Archiver::LoadIDs(ids);
    for(const auto &i : ids) {
	    Archive tempArchive;
	    if(Archiver::LoadArchive(i, tempArchive)<0) {
		    continue;
	    }
    ServerModule tempModule(tempArchive);
	    modules.push_back(tempModule);
    }
    l.logFinish(((float)modules.size() / ids.size()) > 0.10 ? 0 : -1); // If over 90% of servers are loaded and initialized correctly

    l.logLengthyFunction("Core: Booting up TCP/IP Daemon...");
    int ret = netcom.SetUpListener();
    l.logFinish(ret);

    std::cout << "Core: ----- SWIRVE FRAMEWORK BOOTUP FINISHED -----\n";
    std::cout << "Swirve Framework is now online. Going into TCP Request answer loop...\n";
    fflush(stdout);

    while(true) {
	sleep(2);
	std::vector<Connection> _connections;
	netcom.ReadIncomingConnections(_connections);
	for(const auto &i : _connections) {
	    std::cout << "Core: Connection at " << i.ip << " with fd " << i.sockfd << " has data ->" << i.buffer << "<- in buffer.\n";
	}
	fflush(stdout);
    }


    sleep(5);
    std::cout << "Core: Press enter to shut down Swirve Framework.\n";
    getchar();
    
    
    l.logLengthyFunction("Core: Stopping TCP/IP Server Daemon...");
    l.logFinish(netcom.StopListener());
    std::cout << "Core: Return\n";
    return;
}


int main(void) {
    core_entry();
    return 0;
}
