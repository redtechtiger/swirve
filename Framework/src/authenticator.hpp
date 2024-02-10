#ifndef AUTHENTICATOR_H
#define AUTHENTICATOR_H

#include <vector>
#include <string>

class Authenticator {
    
    public:

        static std::vector<std::string> GetPublicCredentials();
        static std::vector<std::string> GetAuthenticKeys();
        static int PushCredential(std::string);
        static int EraseCredential(std::string);
        static int PushAuthenticKey(std::string);
        static int EraseAuthenticKey(std::string);

        static int AuthenticateKey(std::string);

};

#endif