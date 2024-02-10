// Header
#include "sysutils.hpp"

// Includes
#include <string>
#include <vector>
#include <fstream>
#include <iostream>
#include <sstream>
#include <unistd.h>
#include <thread>

// Defines
#define PROCMEMINFOLOC "/proc/meminfo"
#define PROCSTATLOC "/proc/stat"
#define PROCPIDBEGIN "/proc/"
#define PROCPIDEND "/stat"
#define CPUDELTATIME 1
#define CLKTCK 100

// Namespaces
using  namespace std;

template <typename T>
void print_vector(T couts) {
    for(const auto &i : couts) {
	    std::cout << i << "\t";
    }
    std::cout << std::endl;
}

SysUtils::SysUtils() {

}

SysUtils::~SysUtils() {
    StopSync();
}

int SysUtils::GetSystemCores() {
    return (int)std::thread::hardware_concurrency();
}

int SysUtils::GetSystemMemory() {
    string label;
    string token;
    ifstream statmstream(PROCMEMINFOLOC);
    statmstream >> label >> token;
    try {
        return stoi(token)/1000; // From kB to MB
    } catch (...) {
        return -1; // Framework may fail to read /proc/meminfo kernel data and tokenize it as null causing stoi() to fail. Return -1 in this case.
    }
}

vector<int> SysUtils::readstat() {
    ifstream procstat;
    procstat.open(PROCSTATLOC);
    if(!procstat.is_open()) return vector<int>(); // Return empty vector if fail
    string procstatcontent;
    getline(procstat, procstatcontent);
    procstat.close();
    stringstream buffer(procstatcontent);
    string value;
    vector<int> output;
    getline(buffer, value, ' ');
    while(getline(buffer, value, ' ')) {
        try {
            output.push_back(stoi(value));
        } catch (...) { }
    }
    return output;
}

vector<int> SysUtils::readstat(int pid) {
    ifstream procstat;
    procstat.open(PROCPIDBEGIN + to_string(pid) + PROCPIDEND);
    if(!procstat.is_open()) return vector<int>(); // Return empty vector if fail
    string procstatcontent;
    getline(procstat, procstatcontent);
    procstat.close();
    stringstream buffer(procstatcontent);
    string value;
    vector<int> output;

    getline(buffer, value, ' ');
    while(getline(buffer, value, ' ')) {
        try {
            output.push_back(stoi(value));
        } catch (...) { }
    }
    return output;
}

int SysUtils::GetSystemCPU() {
    vector<int> cpuvalsfirst = readstat();
    sleep(CPUDELTATIME);
    vector<int> cpuvalssecond = readstat();
    vector<int> cpuvalsdelta;
    for (size_t i=0;i<cpuvalsfirst.size();i++) {
        cpuvalsdelta.push_back(cpuvalssecond[i]-cpuvalsfirst[i]); // Calculate delta vector between 1st and 2nd measurement
    }
    int sum = 0;
    for (const auto &i : cpuvalsdelta) {
        sum += i;
    }

    if(sum==0) {
        cout << "SKIPPED UNDEFINED BEHAVIOUR!!!" << endl;
        return -1;
    }
    return 100 - (cpuvalsdelta[3]*100 / sum);
}

int SysUtils::GetPIDCPU(int pid) {
    vector<int> cpuvals = readstat(pid); // proc/<pid>/stat
    if(cpuvals.empty()) {
        sleep(CPUDELTATIME);
        return -1;
    };
    int totaltimefirst = cpuvals[10] + cpuvals[11]; // Fetch utime and stime
    sleep(CPUDELTATIME);
    cpuvals = readstat(pid); // Refresh values
    if(cpuvals.empty()) return -1;
    int totaltimesecond = cpuvals[10] + cpuvals[11];
    int totaldeltatime = totaltimesecond - totaltimefirst; // Calculate delta execution time

    double totaldeltaseconds = (double)totaldeltatime / CLKTCK; // Convert from system ticks to seconds
    // totaldeltaseconds is the amount of seconds that the PID has spent in execution the last CPUDELTATIME s.

    int utilisation = totaldeltaseconds*100 / CPUDELTATIME; // Get percent

    return utilisation / GetSystemCores();
}

int SysUtils::GetSystemMEM() {
    ifstream meminfo((string)"/proc/meminfo");
    if(!meminfo.good()) return -1;
    string memTotal, memTotalKb, labelKb, memFree, memFreeKb;
    meminfo >> memTotal >> memTotalKb >> labelKb >> memFree >> memFreeKb;
    meminfo.close();
    try {
        return stoi(memFreeKb)/1024;
    } catch (...) {
        return -1;
    }
}

int SysUtils::GetPIDVmRSS(int pid) {
    ifstream statm((string)"/proc/" + to_string(pid) + "/statm"); // Open statm file
    if(!statm.good()) return -1;
    string VmSize, VmRSS;
    statm >> VmSize >> VmRSS;
    statm.close();
    try {
        return stoi(VmRSS)/1024; // Megabytes
    } catch (...) {
        return -1; // Stoi failed
    }
}

int SysUtils::GetPIDVmSize(int pid) {
    ifstream statm((string)"/proc/" + to_string(pid) + "/statm");
    if(!statm.good()) return -1;
    string VmSize;
    statm >> VmSize;
    statm.close();
    try {
        return stoi(VmSize)/1024; // Megabytes
    } catch (...) {
        return -1; // Stoi failed
    }
}

void SysUtils::getAsyncVmRSS(bool& run, int pid, int& value) {
    while(run) {
        value = GetPIDVmRSS(pid);
        sleep(CPUDELTATIME);
    }
}

void SysUtils::getAsyncVmSize(bool& run, int pid, int& value) {
    while(run) {
        value = GetPIDVmSize(pid);
        sleep(CPUDELTATIME);
    }
}

void SysUtils::getAsyncCPU(bool& run, int pid, int& value) {
    while(run) {
        value = GetPIDCPU(pid);
    }
}

void SysUtils::getAsyncMemSys(bool& run, int& value) {
    while(run) {
        value = GetSystemMEM();
        sleep(CPUDELTATIME);
    }
}

void SysUtils::getAsyncCPUSys(bool& run, int& value) {
    while(run) {
      value = GetSystemCPU();
    }
}

int SysUtils::StartAsyncVmSize(int pid, int& value) {
    runAsync = true;
    thread_vmsize_pid_running = true;
    thread_vmsize_pid = thread(&SysUtils::getAsyncVmSize, this, ref(runAsync), pid, ref(value));
    return 0;
}

int SysUtils::StartAsyncVmRSS(int pid, int& value) {
    runAsync = true;
    thread_vmrss_pid_running = true;
    thread_vmrss_pid = thread(&SysUtils::getAsyncVmRSS, this, ref(runAsync), pid, ref(value));
    return 0;
}

int SysUtils::StartAsyncCPU(int pid, int& value) {
    runAsync = true;
    thread_cpu_pid_running = true;
    thread_cpu_pid = thread(&SysUtils::getAsyncCPU, this, ref(runAsync), pid, ref(value));
    return 0;
}

int SysUtils::StartAsyncMemSys(int& value) {
    runAsync = true;
    thread_mem_system_running = true;
    thread_mem_system = thread(&SysUtils::getAsyncMemSys, this, ref(runAsync), ref(value));
    return 0;
}

int SysUtils::StartAsyncCPUSys(int& value) {
    runAsync = true;
    thread_cpu_system_running = true;
    thread_cpu_system = thread(&SysUtils::getAsyncCPUSys, this, ref(runAsync), ref(value));
    return 0;
}

int SysUtils::StopSync() {

    runAsync = false;
    if(thread_cpu_system_running) thread_cpu_system.join();
    if(thread_cpu_pid_running) thread_cpu_pid.join();
    if(thread_mem_system_running) thread_mem_system.join();
    if(thread_vmrss_pid_running) thread_vmrss_pid.join();
    if(thread_vmsize_pid_running) thread_vmsize_pid.join();

    thread_cpu_pid_running = false;
    thread_cpu_system_running = false;
    thread_mem_system_running = false;
    thread_vmrss_pid_running = false;
    thread_vmsize_pid_running = false;

    return 0;
}
