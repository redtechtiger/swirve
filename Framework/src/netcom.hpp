#ifndef NETCOM_H
#define NETCOM_H

// Defines
#define PORT 12555 // Default port when new config is generated
#define PORTCONFIGPATH "./data/portconfig.bin"
#define EXTIPSHELLEXECUTE "curl --silent ifconfig.me"

// Types
#include <string>
#include <vector>
#include <thread>
#include <chrono>
#include <map>

// Network communication
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netdb.h>

// Data type for each conection
struct Connection {
    int sockfd;
    struct sockaddr sockaddrdata;
    socklen_t sockaddrlen;
    std::string ip;
    std::string buffer;
};



class NetworkCommunicator {
    private:
		std::vector<Connection> connections;
		int lSocket;
		void serverLoop(bool* shouldStop);
		int stopServerLoop(); // Sets "stopping" and joins thread

		std::string extIp;
		std::string port;
		bool stopping;

		std::thread tcpDaemon;

    public:
		int GeneratePortConfig(const int* port);
		int SetUpListener(); // Start socket & set it to listen
		int StopListener(); // Delete listening socket
		int KillConnections();

		int GetIp(std::string &external_ip_out);
		int GetPort(std::string &port_out);

		int readPortConfig(int &port);
		int ReadIncomingConnections(std::vector<Connection> &connections, std::map<int, bool>* auth); // Get vector of <Connection>
		int WriteIncomingConnection(const int sockfd, const std::string buffer); // Write to Connection
		int CloseConnection(const int socketfd);

};

#endif