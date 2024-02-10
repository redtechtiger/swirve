#ifndef SYSUTILS_H
#define SYSUTILS_H

#include <vector>
#include <thread>
#include "log.hpp"

class SysUtils {

    private:
        static std::vector<int> readstat();
        static std::vector<int> readstat(int pid);

        bool runAsync = false;

        void getAsyncVmRSS(bool& run, int pid, int& value);
        void getAsyncVmSize(bool& run, int pid, int& value);
        void getAsyncCPU(bool& run, int pid, int& value);
        void getAsyncMemSys(bool& run, int& value);
        void getAsyncCPUSys(bool& run, int& value);

        std::thread thread_vmrss_pid;
        bool thread_vmrss_pid_running = false;
        std::thread thread_vmsize_pid;
        bool thread_vmsize_pid_running = false;
        std::thread thread_cpu_pid;
        bool thread_cpu_pid_running = false;
        std::thread thread_mem_system;
        bool thread_mem_system_running = false;
        std::thread thread_cpu_system;
        bool thread_cpu_system_running = false;

        Logger l;

    public:
        SysUtils();
        ~SysUtils();

        static int GetSystemCores();
        static int GetSystemMemory();

        static int GetSystemCPU();
        static int GetPIDCPU(int pid);
        static int GetSystemMEM();
        static int GetPIDVmRSS(int pid);
        static int GetPIDVmSize(int pid);

        int StartAsyncVmRSS(int pid, int& value);
        int StartAsyncVmSize(int pid, int& value);
        int StartAsyncCPU(int pid, int& value);

        int StartAsyncMemSys(int& value);
        int StartAsyncCPUSys(int& value);

        int StopSync();

};


#endif