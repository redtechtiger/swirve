// Includes
#include <string>
#include <iostream>

// Class
class SysInfo {

// Defines
#define MAX_KERNEL_SIZE 1024
#define MAX_UPTIME_SIZE 1024

public:
    static std::string GetKernelName();
    static std::string GetKernelUptime();
    static std::string GetFrameworkName();
    static std::string GetFrameworkAPI();

    static std::string GetDeployOrganization();

};
