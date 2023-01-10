// Header file
#include "netcom.h"

// Types
#include <string>
#include <vector>

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

int NetworkCommunicator::serverLoop() { // Function called by fork
    while(!stopping) {
	Connection iConnection;
	cout << "TCPDAEMON: WAITING FOR CONNECTION..." << endl;
	// fflush(stdout);
	iConnection.sockfd = accept(lSocket, iConnection.sockaddrdata, iConnection.sockaddrlen);
	if(iConnection.sockfd<0) return -1; // Error
	char ipBuffer[INET_ADDRSTRLEN];
	inet_ntop(AF_INET, iConnection.sockaddrdata, ipBuffer, INET_ADDRSTRLEN);
	iConnection.ip = string(ipBuffer);
	connections.push_back(iConnection); // Add connection to vector
	sleep(1);
    }
    return 0;
}

int NetworkCommunicator::stopServerLoop() {
    // TODO : Set NetworkCommunicator::stopping to True; if thread hasn't exited after x defined seconds, kill it through SIGTERM signal
    stopping=true;
    sleep(2);
    
    return 0; // Not implemented yet! TODO : FIX ERROR HANDLING
}

int NetworkCommunicator::SetUpListener() {
    int ret = -1; // Return code for calls further
		  
    // Create listening socket : IPv4, TCP/IP, auto
    lSocket = socket(AF_INET, SOCK_STREAM, 0);
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

    // Bind the listening socket to the sockaddr_in ( hint )
    ret = bind(lSocket, (struct sockaddr*)&sock_addr, sizeof(sock_addr));
    if(ret<0) return -1;

    // Mark the listening socket as listening
    ret = listen(lSocket, SOMAXCONN);
    if(ret<0) return -1;

    stopping = false; // Make sure server doesn't exit immediately

    cout << "Info: Launching server..." << endl;
    // Fork & start serverloop() on another thread for non-blocking accept()
    int forkPid = fork();
    if(forkPid<0) { // Failed to start thread to handle connections
	
	StopListener();
	KillConnections();
	return -1;
    } else if (forkPid>0) { // Parent ( forkPid = pid of fork )
	forkThreadPid = forkPid; // Assign fork PID ( for emergency killing )
	return 0;
    } else if (forkPid==0) { // Child (server script)
	// TODO : ADD PROPER ERROR HANDLING
	serverLoop();
	cout << "Warn: TCPServer loop exited, killing thread..." << endl;
	exit(0);
	return 0;
    }

    cerr << "FATAL: EXHAUSTIVE IF STATEMENT NOT EXHAUSTIVE, CLOSING SOCKET" << endl;
    StopListener();
    cerr << "SOCKET CLOSED, THROWING EXCEPTION..." << endl;
    throw runtime_error("Exhaustive if-statement not exhaustive ( netcom.cpp @~100 ) ");
    return 0;
    
}

int NetworkCommunicator::ReadIncomingConnections(std::vector<Connection> &connections) {
    
    for(int index=0;index<connections.size();index++) {
	Connection* connection = &connections[index];
	char _buffer[4096];
	int _bytesRead = static_cast<int>(recv(connection->sockfd, _buffer, sizeof(_buffer), MSG_DONTWAIT));
        if(_bytesRead<0) { // Read failed or nothing to read
	    if(errno!=EAGAIN||errno!=EWOULDBLOCK) { // Read failed
		return -1;
	    }
        } else if(_bytesRead==0) { // Connection closed
	    connections.erase(connections.begin()+index); // Delete from vector
	} else { // Bytes have been read
	    connection->buffer = string(_buffer);
	}
    }
    return -1; // Return error as function isn't implemented yet
}

int NetworkCommunicator::WriteIncomingConnection(const int id, const std::string buffer) {
    // TODO : Send data according to file descriptor ID
    
    return -1; // Return error as function isn't implemented yet
}

int NetworkCommunicator::StopListener() {
    int ret = -1;
    ret = stopServerLoop();
    if(ret<0) return -1;
    close(lSocket);
    return close(lSocket);
}

int NetworkCommunicator::KillConnections() {

    // TODO : Iterate through vector of connections and close all of them forcefully, return 0 if all connections successfully closed.

    return -1; // Not implemented
}
