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
    std::cout << "Starting Minecraft Server ";
    fflush(stdout);
    sleep(1);
    std::cout << "[" << (handler.Start() == 0 ? "\u001b[32mOK\u001b[37m" : "\u001b[31mFAIL\u001b[37m") << "]\n";
    std::cout << "Polling server...\n";
    std::string output;
    while(1==1) {
	//handler.GetLog(output);
	std::cout << "\rServer State: " << handler.State();
	fflush(stdout);
	sleep(1);
    }
}




int main(void) {
    core_entry();
    return 0;
}
