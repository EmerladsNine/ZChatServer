#pragma once
#include <vector>
#include <unistd.h>
#include <sys/socket.h>
#include <cstdio>
#include <cstdlib>
#include "networking_manager.h"

class Client
{
private:
public:
        NetworkingManager *networkingManager;
        bool isAlive;
        int fd;
        std::vector<char> buf;
        bool operator==(const Client &other) const
        {
                return fd == other.fd; // or whatever defines equality
        }
        void read();

        Client(NetworkingManager *networkingManager);
        ~Client();
};