#include <iostream>
#include <unistd.h>
#include "../handler/handler.h"
#include "../modules/module.h"
#include "../archive/archive.h"
#include "../logger/log.h"

void showLogo() {
    std::cout << "===================================================================================================\n";
    std::cout << "\n";
    std::cout << "  _________       .__                     \n";
    std::cout << " /   _____/_  _  _|__|_________  __ ____  \n";
    std::cout << " \\_____  \\\\ \\/ \\/ /  \\_  __ \\  \\/ // __ \\ \n";
    std::cout << " /        \\\\     /|  ||  | \\/\\   /\\  ___/ \n";
    std::cout << "/_______  / \\/\\_/ |__||__|    \\_/  \\___  >\n";
    std::cout << "        \\/                             \\/\n";
    std::cout << "						Framework 1.0.0-ALPHA.1\n";
    std::cout << "===================================================================================================\n";
}

void shell_loop(MinecraftHandler &handler) {
    std::string input;
    std::string output;
    while(1==1) {
	std::cout << ">";
	std::getline(std::cin,input);
	if(input=="help") {
	    std::cout << "[Shell]: Commands: help, log, status, stop, start, reboot, kill\n";
	} else if(input=="log") {
	    std::cout << "[Server Output]:\n";
	    handler.GetLog(output);
	    std::cout << output << "\n";
	} else if(input=="status") {
	    std::cout << "[Shell]: Server status [" << handler.State() << "]\n";
	} else if(input=="start") {
	    std::cout << "[Shell]: Server start " << (handler.Start() == 0 ? "OK" : "FAIL") << "\n";
	} else if(input=="stop") {
	    std::cout << "[Shell]: Server stop " << (handler.Stop() == 0 ? "OK" : "FAIL") << "\n";
	} else if(input=="reboot") {
	    std::cout << "[Shell]: Server reboot " << (handler.Restart() == 0 ? "OK" : "FAIL") << "\n";
	} else if(input=="kill") {
	    std::cout << "[Shell]: Server kill " << (handler.Kill() == 0 ? "OK" : "FAIL") << "\n";
	} else if(input=="exit") {
	    std::cout << "[Shell]: Bye\n";
	    break;
	} else {
	    std::cout << "[Shell]: Treating command as server input\n";
	    handler.SendCommand(input);
	}
	fflush(stdout);
    }
}

template <typename T>
void print_vector(T couts) {
    for(auto i : couts) {
	std::cout << i << "\n";
    }
}

void core_entry() {
    std::cout << "Initiating framework core...\n";
    Logger l;
    showLogo();
    std::cout << "Starting services...\n";
    
    // std::cout << "[" << (handler.Start() == 0 ? "\u001b[32mOK\u001b[37m" : "\u001b[31mFAIL\u001b[37m") << "]\n";
    
    std::vector<unsigned long> v;
    l.logFunction("Loading IDs...", Archiver::LoadIDs(v));

    std::vector<ServerModule> modules;
    int loadedModules;
    int fail = 0;

    l.logLengthyFunction("Loading modules...");

    for(auto i : v) {
	Archive tempArchive;
	if(Archiver::LoadArchive(i,tempArchive)==0) {
	    ServerModule module(tempArchive);
	    modules.push_back(module);
	    loadedModules++;
	} else fail = 1;
    }

    l.logFinish(fail);

    std::cout << "Server boot finish, loaded " << loadedModules << " out of " << v.size() << " server modules.\n";


    std::cout << "Finish\n";
    
    return;    
}


int main(void) {
    core_entry();
    return 0;
}
