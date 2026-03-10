#pragma once
#include <vector>
#include <unistd.h>
#include <sys/socket.h>
#include <cstdio>
#include <cstdlib>
#include "services.h"
#include "session.h"

class Client
{
private:
public:
        bool isAlive = true;
        bool canRename;
        Session session;
        bool isAuthenticated = false;
        int fd;
        std::vector<char> buf;
        bool operator==(const Client &other) const
        {
                return fd == other.fd;
        }
        void read(Services &services);
        void authenticate(Services &services, uint32_t id, std::string &accessTokenHash);
        void disconnect(Services &services);
        Client();
        ~Client();
};