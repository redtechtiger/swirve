// Header
#include "sysinfo.hpp"

// Includes
#include <string>
#include <iostream>
#include <filesystem>
#include <stdio.h>

// Information
#include "stats.h"

std::string trim_end(std::string input) {
    if (input.length() > 0 && input[input.length()-1] == '\n') {
        return input.erase(input.length() - 1);
    } else {
        return input;
    }
}

std::string SysInfo::GetKernelName() {
     
    FILE* file_ptr = popen("uname -r","r");
    if (file_ptr == nullptr)
        return "Failure in kernel name resolution";

    char buf[MAX_KERNEL_SIZE] = {0};
    fgets(buf, sizeof(buf)-1, file_ptr);

    return trim_end(std::string(buf));
}

std::string SysInfo::GetKernelUptime() {

    FILE* file_ptr = popen("uptime -p", "r");
    if(file_ptr == nullptr)
        return "Failure in kernel uptime resolution";

    char buf[MAX_UPTIME_SIZE] = {0};
    fgets(buf, sizeof(buf)-1, file_ptr);

    return trim_end(std::string(buf));
}

std::string SysInfo::GetFrameworkName() {
    
    return std::string(FRAMEWORK_NAME) + std::string(" ") + std::string(FRAMEWORK_VERSION);
}

std::string SysInfo::GetFrameworkAPI() {

    return std::to_string(API_VERSION_MAJOR) + (std::string)"." + std::to_string(API_VERSION_MINOR);
}

std::string SysInfo::GetDeployOrganization() {
    return DEPLOY_ORGANIZATION;
}

