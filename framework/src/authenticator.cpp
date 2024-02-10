// Header
#include "authenticator.hpp"

// Includes
#include <string>
#include <vector>
#include <iostream>
#include <fstream>

// Defines
#define KEYFILE "./data/keys.bin"
#define CREDFILE "./data/credentials.bin"

// Namespaces
using namespace std;

vector<string> Authenticator::GetPublicCredentials() {
    ifstream credStream(CREDFILE);
    if(!credStream.good()) return vector<string>({"-1"});
    string credential;
    vector<string> credentials;
    while(credStream >> credential) {
        credentials.push_back(credential);
    }
    credStream.close();
    return credentials;
}

int Authenticator::PushCredential(string credential) {
    ofstream credStream(CREDFILE, ios::app);
    if(!credStream.good()) return -1;
    credStream << credential << " ";
    credStream.close();
    return 0;
}

int Authenticator::EraseCredential(string credential) {
    vector<string> credentials = GetPublicCredentials();
    if(credentials.size()<=0 || credentials[0]=="-1") return -1;
    bool erased = false;
    for(size_t i=0; i<credentials.size(); i++) {
        if(credentials[i]==credential) {
            credentials.erase(credentials.begin()+i);
            erased = true;
        }
    }
    return erased;
}

vector<string> Authenticator::GetAuthenticKeys() {
    ifstream keyStream(KEYFILE);
    if(!keyStream.good()) return vector<string>({"-1"}); // Return empty vector if fail!
    string key;
    vector<string> keys;
    while(keyStream >> key) {
        keys.push_back(key);
    }
    keyStream.close();
    return keys;
}

int Authenticator::PushAuthenticKey(string key) {
    ofstream keyStream(KEYFILE, ios::app);
    if(!keyStream.good()) return -1; 
    keyStream << key << " ";
    keyStream.close();
    return 0;
}

int Authenticator::EraseAuthenticKey(string key) {
    vector<string> keys = GetAuthenticKeys();
    if(keys.size()<=0 || keys[0]=="-1") return -1;
    bool erased = false;
    for (size_t i=0; i<keys.size(); i++) {
        if(keys[i]==key) {
            keys.erase(keys.begin()+i);
            erased = true;
        }
    }
    return erased;
}

int Authenticator::AuthenticateKey(string key) {
    cout << endl;
    vector<string> keys = GetAuthenticKeys();
    if(keys[0]=="-1") return -1;
    for(const auto &validkey : keys) {
        if(validkey==key) return 0;
    }
    return -1;
}