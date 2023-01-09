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
    std::cout << "Main: Initiating framework core...\n";
    Logger l;
    showLogo();
    std::cout << "Main: Starting services...\n"; 
    NetworkCommunicator netcom;

    std::cout << "Main: ---- Launching test: TCP/IP server on IP:PORT ->[NOT IMPLEMENTED]" << "<- ----\n";
    int ret = netcom.SetUpListener();

    if(ret<0) {
	std::cout << "Main: ---- Test failed with NETCOM returncode " << ret << ". Program will abort. ----\n";
    } else {
	std::cout << "Main: ---- Test PASS with returncode " << ret << "! ----\n";
    }

    std::cout << "Main: Return in 5 seconds\n";
    sleep(5);
    std::cout << "Main: Return\n";
    return;
}


int main(void) {
    core_entry();
    return 0;
}
