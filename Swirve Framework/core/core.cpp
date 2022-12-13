#include <iostream>
#include <unistd.h>
#include "../loader/async_loader.h"


void core_entry() {
    std::cout << "Initiating core server...\n";
}




int main(void) {
    core_entry();
    return 0;
}