#include <iostream>
#include <unistd.h>
#include "../handler/handler.h"

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
    sleep(2);
    showLogo();
    std::cout << "Starting services...\n";
    //std::cout << "Starting Minecraft Server ";
    fflush(stdout);
    sleep(1);
    //std::cout << "[" << (handler.Start() == 0 ? "\u001b[32mOK\u001b[37m" : "\u001b[31mFAIL\u001b[37m") << "]\n";
    std::cout << "Initiating Swirve Framework Shell...\n";
    std::string input;
    std::string output;
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
