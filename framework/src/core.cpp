#include <iostream>
#include <unistd.h>
#include "handler.hpp"
#include "sysinfo.hpp"
#include "module.hpp"
#include "archive.hpp"
#include "netcom.hpp"
#include "log.hpp"
#include "parser.hpp"
#include "sysutils.hpp"
#include "authenticator.hpp"
#include <memory>
#include <fstream>
#include <string>
#include <iomanip>
#include <map>
#include <cmath>

#define ASCIIGREEN "\u001b[32m"
#define ASCIICYAN "\u001b[36m"
#define ASCIIRESET "\u001b[0m"

#define _DEBUG

using namespace std;

void showLogo() {
    ifstream buildnr("buildnumber"); // Get build number
    string buf;
    getline(buildnr, buf);

    cout << "---===--- Swirve Framework v1.2-DEV" << buf << " ---===---\n";
    cout << "RedTechTiger Media, all rights reserved.\n\n";
}

#ifdef _DEBUG
template <typename T>
void print_vector(T couts) {
    for(const auto &i : couts) {
	    cout << i << "\t";
    }
    cout << endl;
}
#endif

#ifdef _DEBUG
void consoletest() {
    for(int i=0; i<50; i++) {
        cout << "\u001b[" + to_string(i) + "m" << i << ASCIIRESET;
    }
}
#endif

int core_entry(vector<string> args) {

    flags FLAGS;
    for(auto const &arg : args) {
        if(arg=="--ignoreinfo") FLAGS.IGNOREINFO = true;
        if(arg=="--ignorewarn") FLAGS.IGNOREWARN = true;
        if(arg=="--ignorefatal") FLAGS.IGNOREFATAL = true;
    }
    Logger l(FLAGS);

    l.info("Core", "Booting Swirve Framework...");


    // Set up classes
    NetworkCommunicator netcom;
    map<unsigned long, shared_ptr<ServerModule>> modules;


    // Fetch server information and make sure router is accessible
    l.info("Core","Fetching system information...");

    string ip;
    int ip_ret = netcom.GetIp(ip);
    #ifdef _DEBUG
    ip_ret=0;
    #endif
    string cores = to_string(SysUtils::GetSystemCores());
    string memory = to_string(SysUtils::GetSystemMemory()/1000); // In gigabytes

    l.info("Core", (string)"      * Processor: "+ASCIICYAN+cores+" cores"+ASCIIRESET);
    l.info("Core", (string)"      * Memory:    "+ASCIICYAN+memory+" GB"+ASCIIRESET);
    l.info("Core", (string)"      * Ext. IP:   "+ASCIICYAN+ip+ASCIIRESET);
    fflush(stdout);

    if(ip_ret!=0&&!FLAGS.IGNOREFATAL) {
        l.fatal("Core", "Error PRBO_IPVA_NZRO_FAIL: Failed to fetch router ip!");
        l.fatal("Core", "Framework powering down");
        return -1;
    }


    // Load all server modules
    l.infofuncstart("Core", "Building server modules...");

    vector<unsigned long> ids;
    Archiver::LoadIDs(ids);
    for(const auto &i : ids) {
	    Archive tempArchive;
	    if(Archiver::LoadArchive(i, tempArchive)<0) {
		    continue;
	    }
        shared_ptr<ServerModule> tempModule(new ServerModule(tempArchive));
	    modules[tempModule->ID] = tempModule;
    }
    int returnvalue = l.infofuncreturn(modules.size() == ids.size() ? 0 : 1); // Fail if all modules not loaded correctly
    if(returnvalue!=0&&!FLAGS.IGNOREFATAL) {
        l.fatal("Core", "Error BOOT_MODL_NZRO_FAIL: Failed to load server modules!");
        l.fatal("Core", "Framework powering down");
        return -1;
    }
    


    // Load authorised IDs
    l.infofuncstart("Core", "Loading authorised keys...");
    vector<string> authenticKeys = Authenticator::GetAuthenticKeys();

    if(authenticKeys.empty()&&!FLAGS.IGNOREWARN) {
        l.infofuncreturn(0);
        l.warn("Core", "No keys are currently authorised.");
    } else if(authenticKeys[0]=="-1"&&!FLAGS.IGNOREFATAL) {
        l.infofuncreturn(-1);
        l.fatal("Core", "Error BOOT_ATKE_NZRO_FAIL: Failed to load authorised keys!");
        l.fatal("Core", "Framework powering down");
        return -1;
    } else {
        l.infofuncreturn(0);
    }

    l.infofuncstart("Core", "Starting system monitoring...");
    SysUtils sysutil;
    int systemram = 0;
    returnvalue = sysutil.StartAsyncCPUSys(systemram);
    l.infofuncreturn(returnvalue);
    if(returnvalue!=0&&!FLAGS.IGNOREWARN) {

    }


    // Start netcom server
    l.infofuncstart("Core", "Launching TCP/IP server...");
    returnvalue = l.infofuncreturn(netcom.SetUpListener());
    if(returnvalue!=0&&!FLAGS.IGNOREFATAL) {
        l.fatal("Core", "Error BOOT_NCOM_NZRO_FAIL: Failed to start netcom server!");
        l.fatal("Core", "Framework powering down");
        return -1;
    }

    // Start parser server
    l.infofuncstart("Core", "Launching ActiveParser server...");
    unique_ptr<ActiveParser> parser(new ActiveParser(&netcom,&modules));
    returnvalue = l.infofuncreturn(parser->StartParser());
    if(returnvalue!=0&&!FLAGS.IGNOREFATAL) {
        l.fatal("Core", "Error BOOT_ACPA_NZRO_FAIL: Failed to start activeparser server!");
        l.fatal("Core", "Framework powering down");
        netcom.StopListener();
        return -1;
    }


    // Bootup finished
    l.info("Core","");
    l.info("Core", " -- Framework Online --");

    while(true) {
        sleep(5);
	// Clear server log pipes
	for(auto& module : modules) {
        std::string _temp = std::string();
	    module.second->GetLog(_temp);
	}
        if(parser->ParserException()==-1) {
            l.fatal("Core","Detected exception in ActiveParser!");
            l.warn("Core","Rebooting network services...");
            l.info("Core","-- Core Rescue Tool --");
            l.info("Core","Stopping ActiveParser...");
            parser->StopParser();
            l.info("Core","Stopping NetworkCommunicator...");
            netcom.StopListener();
            l.info("Core","Network services stopped");
            bool started = false;
            while(!started) {
                sleep(5);
                l.info("Core","Reinitializing NetworkCommunicator...");
                netcom = NetworkCommunicator();
                l.info("Core","Booting Network Communicator...");
                int ret = netcom.SetUpListener();
                if(ret!=0) {
                    l.fatal("Core","Netcom boot FAIL, retrying");
                    netcom.StopListener();
                    continue;
                } else {
                    l.info("Core", "Netcom boot OK");
                }
                l.info("Core","Reinitializing ActiveParser...");
                parser.reset(new ActiveParser(&netcom, &modules));
                l.info("Core","Booting ActiveParser...");
                ret = parser->StartParser();
                if(ret!=0) {
                    l.fatal("Core","ActiveParser boot FAIL, retrying");
                    netcom.StopListener();
                    parser->StopParser();
                    continue;
                } else {
                    l.info("Core", "ActiveParser boot OK");
                }
                l.info("Core","-- Core Rescue Tool: Framework restored successfully --");

            }
        }
    }

    l.warn("Core", "Framework going down!");

    // Code never gets here as Framework is stuck in above while(true) loop.
    sleep(5);
    l.infofuncstart("Core","Stopping system monitoring...");
    sysutil.StopSync();
    l.infofuncreturn(0);

    l.infofuncstart("Core", "Stopping ActiveParser server...");
    l.infofuncreturn(parser->StopParser());
    l.infofuncstart("Core", "Stopping TCP/IP server...");
    l.infofuncreturn(netcom.StopListener());
    l.info("Core","Destructing stack cache...");
    return 0;
}

int main(int argc, char** argv) {
    showLogo();
    int frameworkresult = core_entry(vector<string>(argv, argv + argc)); // Create vector: start at memory address of ARGV, end at ARGV + ARGC (argument count)
    cout << "Core[INFO] Framework stopped\n\n";
    return frameworkresult;
}
