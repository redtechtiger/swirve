#include <iostream>
#include <unistd.h>
#include "../handler/handler.h"
#include "../modules/module.h"
#include "../archive/archive.h"

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

void core_entry() {
    std::cout << "Initiating framework core...\n";
    MinecraftHandler handler;
    showLogo();
    std::cout << "Starting services...\n";
    //std::cout << "Starting Minecraft Server ";
    fflush(stdout);
    // std::cout << "[" << (handler.Start() == 0 ? "\u001b[32mOK\u001b[37m" : "\u001b[31mFAIL\u001b[37m") << "]\n";
    // std::cout << "Initiating Swirve Framework Shell...\n";
    std::string input;
    std::string output;
    
    std::cout << "Load Archive...\n";
    int hash = 1337;
    Archive archive;
    Archiver::LoadArchive(hash,archive);

    std::cout << "Set up Server Module...\n";
    ServerModule module1(archive);
    std::cout << "Name: " << module1.Name << "\n";
    std::cout << "LaunchPath: " << module1.LaunchPath << "\n";
    for (auto i : module1.AccessIDs)
	std::cout << "AccessID Found: " << i << "\n";


    // archive.ID = hash;
    // archive.Name = "Jacob's server";
    // archive.AccessIDs.push_back(hash);
    // archive.LaunchPath = "/custom_folder/forge-1.16.5.jar";

    // std::cout << "Writing config...";
    // std::cout << Archiver::SaveArchive(hash, archive) << "\n";
    
    // std::cout << "Reading config...";
    // Archive newArchive;
    // std::cout << Archiver::LoadArchive(hash, newArchive) << "\n";

    // std::cout << "Name: " << newArchive.Name << "\n";
    // std::cout << "LaunchPath: " << newArchive.LaunchPath << "\n";
    // for(auto i : newArchive.AccessIDs)
	    // std::cout << "AccessIDs: " << i << "\n";

    std::cout << "Finish\n";
    return;

    while(1==1) {
	//handler.GetLog(output);
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
	//std::cout << "\rServer State: " << handler.State();
	fflush(stdout);
    }
}


int main(void) {
    core_entry();
    return 0;
}
