// Header file
#include "netcom.h"

// Types
#include <string>
#include <vector>
#include <thread>

// Streams
#include <fstream>
#include <iostream>

// TCP/IP
#include <sys/socket.h> // socket(), AF_INET, SOCK_STREAM, etc
#include <unistd.h> // close(fd)
#include <arpa/inet.h> // inet_pton(str -> binary struct)

// Namespaces
using namespace std;


// Internal func : Read bin config file for port info
int NetworkCommunicator::readPortConfig(int &port) {
    char buffer[10];
    ifstream configStream(PORTCONFIGPATH, ios::binary);
    if(!configStream.good()) return -1;
    configStream.read(buffer,sizeof(buffer));
    port = atoi(buffer); // Generate int from char*
    return 0;
}

int NetworkCommunicator::GetIp(string &external_ip_out) {
    
    FILE* extIpStream = popen(EXTIPSHELLEXECUTE, "r");
    if(!extIpStream) return -1; // Error
    char buffer[128];
    fgets(buffer,sizeof(buffer),extIpStream);
    cout << "Server launching on: " << buffer;
    // TODO : OPEN A PIPE AND READ OUTPUT OF "curl ifconfig.me" ( EXTERNAL IP )
    return -1; // Not yet implemented
}

int NetworkCommunicator::GetPort(string &port_out) {
    port_out = port;
    return 0;
}

int NetworkCommunicator::GeneratePortConfig(const int* port) {
    int dPort = (port == nullptr ? PORT : *port); // Choose default port ( PORT ) if *port is null
    string dPortStr = to_string(dPort);
    ofstream configStream(PORTCONFIGPATH,ios::binary);
    if(!configStream.good()) return -1;
    configStream.write(dPortStr.c_str(),sizeof(dPortStr.c_str()));
    return 0;
}

void NetworkCommunicator::serverLoop(bool* shouldStop) { // Function called by fork
    while(!*shouldStop) {
	sleep(1); // DO NOT MOVE! CONTINUE COMMAND IGNORES SLEEP AT BOTTOM OF LOOP
	Connection iConnection;
	iConnection.sockfd = accept(lSocket, iConnection.sockaddrdata, iConnection.sockaddrlen);
	if(iConnection.sockfd<0&&errno!=EAGAIN&&errno!=EWOULDBLOCK) { // Error
		cerr << "FATAL: TCPDaemon: The TCP/IP Server ran into an error while accepting incoming client connection requests. Daemon will terminate." << endl;
		return;
	}
	if(iConnection.sockfd<0&&(errno==EAGAIN||errno==EWOULDBLOCK)) continue; // No incoming
	char ipBuffer[INET_ADDRSTRLEN];
	inet_ntop(AF_INET, (sockaddr*)&iConnection.sockaddrdata, ipBuffer, INET_ADDRSTRLEN);
	iConnection.ip = string(ipBuffer);
	connections.push_back(iConnection); // Add connection to vector
    }
    return;
}

int NetworkCommunicator::stopServerLoop() {
    stopping=true;
    tcpDaemon.join();
    
    return 0;
}

int NetworkCommunicator::SetUpListener() {
    int ret = -1; // Return code for calls further

    // Create listening socket : IPv4, TCP/IP, auto
    lSocket = socket(AF_INET, SOCK_STREAM | SOCK_NONBLOCK, 0);
    if(lSocket<0) return -1;

    // ( Prep for sockaddr ) Read port from portconfig file ( PORTCONFIGPATH )
    int port;
    ret = readPortConfig(port);
    if(ret<0) return -1;

    // Create a sockaddr_in ( hint ) to assign/bind to the socket later
    struct sockaddr_in sock_addr;
    sock_addr.sin_family = AF_INET; // IPv4
    sock_addr.sin_port = htons(port);
    ret = inet_pton(AF_INET, "0.0.0.0", &sock_addr.sin_addr); // Convert "0.0.0.0" to binary ( 0.0.0.0 = auto)
    if(ret<0) return -1;

    int yes = 1;
    ret = setsockopt(lSocket, SOL_SOCKET, SO_REUSEADDR | SO_REUSEPORT, (void*)&yes, sizeof(yes));
    if(ret<0) return -1;

    // Bind the listening socket to the sockaddr_in ( hint )
    ret = bind(lSocket, (struct sockaddr*)&sock_addr, sizeof(sock_addr));
    if(ret<0) return -1;

    // Mark the listening socket as listening
    ret = listen(lSocket, SOMAXCONN);
    if(ret<0) return -1;

    stopping = false; // Make sure server doesn't exit immediately

    // Fork & start serverloop() on another thread for non-blocking accept()
    tcpDaemon = thread(&NetworkCommunicator::serverLoop,this,&stopping);
    fflush(stdout);
    return 0; 
}

int NetworkCommunicator::ReadIncomingConnections(std::vector<Connection> &connections_out) {
    
    for(int index=0;index<connections.size();index++) {
	Connection* connection = &connections[index];
	char _buffer[4096] = {0};
	int _bytesRead = static_cast<int>(recv(connection->sockfd, _buffer, sizeof(_buffer), MSG_DONTWAIT));
        if(_bytesRead<0) { // Read failed or nothing to read
	    if(errno!=EAGAIN||errno!=EWOULDBLOCK) { // Read failed
		return -1;
	    } else {
		connection->buffer = {0};
		continue;
	    }
        } else if(_bytesRead==0) { // Connection closed
	    connections.erase(connections.begin()+index); // Delete from vector
	} else { // Bytes have been read
	    connection->buffer = {0};
	    connection->buffer = string(_buffer);
	}
    }
    connections_out = connections; // Copy internal connections
    return 0;
}

int NetworkCommunicator::WriteIncomingConnection(const int sockfd, const std::string buffer) {
    for(auto &connection : connections) {
	if(connection.sockfd==sockfd) { // Found connection to write to
	    cout << "NETCOM: Debug: Found socket: Writing to [" << sockfd << "] at " << connection.ip << "\n";
	    int ret;
	    ret = send(connection.sockfd, buffer.c_str(), buffer.size(),0);
	    if(ret<0) {
		cout << "NETCOM: Error: Couldn't write/send to connection.\n";
	    } else if (ret==0){
		cout << "Netcom: Debug: Unexpected input";
	    } else {
		cout << "Successfully wrote " << ret << " bytes of data.\n";
	    }
	}
    }

    return 0;
}

int NetworkCommunicator::StopListener() {
    int ret = -1;
    ret = stopServerLoop();
    if(ret<0) return -1;
    return close(lSocket);
}

int NetworkCommunicator::CloseConnection(const int sockfd) {
    return close(sockfd);
}

int NetworkCommunicator::KillConnections() {

    // TODO : Iterate through vector of connections and close all of them forcefully, return 0 if all connections successfully closed.

    return -1; // Not implemented
}
