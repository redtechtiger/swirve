#include <iostream>
#include <unistd.h>
#include "../handler/handler.h"

void showLogo() {
    std::cout << "===================================================================================================\n";
    std::cout << "  █████████                   ███                                \n";
    std::cout << " ███░░░░░███                 ░░░                                 \n";
    std::cout << "░███    ░░░  █████ ███ █████ ████  ████████  █████ █████  ██████ \n";
    std::cout << "░░█████████ ░░███ ░███░░███ ░░███ ░░███░░███░░███ ░░███  ███░░███\n";
    std::cout << " ░░░░░░░░███ ░███ ░███ ░███  ░███  ░███ ░░░  ░███  ░███ ░███████ \n";
    std::cout << " ███    ░███ ░░███████████   ░███  ░███      ░░███ ███  ░███░░░  \n";
    std::cout << "░░█████████   ░░████░████    █████ █████      ░░█████   ░░██████ \n";
    std::cout << " ░░░░░░░░░     ░░░░ ░░░░    ░░░░░ ░░░░░        ░░░░░     ░░░░░░   Framework 1.0.0-ALPHA.1\n";
    std::cout << "===================================================================================================\n";
}


void core_entry() {
    std::cout << "Initiating core server...\n";
    MinecraftHandler handler;
    sleep(1);
    showLogo();
    std::cout << "CHECKING SYSTEMS...\n";
    std::cout << "MINECRAFT SERVER ";
    fflush(stdout);
    sleep(1);
    std::cout << "[" << (handler.Start() == 0 ? "\u001b[32mOK\u001b[37m" : "\u001b[31mFAIL\u001b[37m") << "]\n";
}




int main(void) {
    core_entry();
    return 0;
}