// Header file
#include "netcom.hpp"
#include "log.hpp"

// Types
#include <string>
#include <vector>
#include <thread>
#include <map>

// Streams
#include <fstream>
#include <iostream>

// TCP/IP
#include <sys/socket.h> // socket(), AF_INET, SOCK_STREAM, etc
#include <unistd.h> // close(fd)
#include <arpa/inet.h> // inet_pton(str -> binary struct)

// Defines
#define CONNECTSLP 1

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
    char buffer[128] = {0};
    fgets(buffer,sizeof(buffer),extIpStream);
    external_ip_out = buffer;
    return external_ip_out == "" ? -1 : 0; // If string is empty, curl failed so return -1
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
    Logger l;
    while(!*shouldStop) {
        sleep(CONNECTSLP); // DO NOT MOVE! CONTINUE COMMAND IGNORES SLEEP AT BOTTOM OF LOOP
        Connection iConnection = Connection {};
        iConnection.sockaddrlen = sizeof(iConnection.sockaddrdata);
        iConnection.sockfd = accept(lSocket, &iConnection.sockaddrdata, &iConnection.sockaddrlen);
        if(iConnection.sockfd<0&&errno!=EAGAIN&&errno!=EWOULDBLOCK) { // Error
            cerr << "TCPDaemon: Warning: The TCP/IP Server ran into an error while accepting incoming client connection requests. Connection skipped." << endl;
            cerr << "TCPDaemon: Error code = " << errno << endl;
            continue;
        }
        if(iConnection.sockfd<0&&(errno==EAGAIN||errno==EWOULDBLOCK)) { // No new incoming connections
            continue;
        }
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

int NetworkCommunicator::ReadIncomingConnections(std::vector<Connection> &connections_out, map<int, bool>* auth) {
    for(size_t index=0;index<connections.size();index++) {
        Connection* connection = &connections[index];
        char _buffer[4096] = {0};
        int _bytesRead = static_cast<int>(recv(connection->sockfd, _buffer, sizeof(_buffer), MSG_DONTWAIT));
        if(_bytesRead<0) { // Read failed or nothing to read
            if(errno!=EAGAIN||errno!=EWOULDBLOCK) { // Read failed
                Logger l;
                l.warn((string)"Netcom","Read failed with errno "+to_string(errno)+(string)" on connection FD "+to_string(connection->sockfd));
                return -1;
            } else {
                connection->buffer = {0};
                continue;
            }
        } else if(_bytesRead==0) { // Connection closed
            (*auth)[connection->sockfd] = false;
            connections.erase(connections.begin()+index); // Delete from vector
            CloseConnection(index);
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
            int ret;
            ret = send(connection.sockfd, buffer.c_str(), buffer.size(),0);
            if(ret<0) {
                Logger l;
                l.warn((string)"Netcom","Write failed with errno "+to_string(errno)+(string)" on connection FD "+to_string(sockfd));
            } else if (ret==0){
                send(connection.sockfd, " ", 1, 0);
            }
        }
    }
    return 0;
}

int NetworkCommunicator::StopListener() {
    KillConnections();
    stopServerLoop();
    return close(lSocket);
}

int NetworkCommunicator::CloseConnection(const int sockfd) {
    return close(sockfd);
}

int NetworkCommunicator::KillConnections() {
    for(const auto &conn : connections) {
        close(conn.sockfd);
    }
    connections.clear();
    return 0;
}
