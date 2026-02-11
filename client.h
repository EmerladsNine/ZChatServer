#pragma once
#include <vector>
#include <unistd.h>
#include <sys/socket.h>
#include <cstdio>
#include <cstdlib>
#include "services.h"

class Client
{
private:
public:
        Services *services;
        bool isAlive;
        bool canRename;
        int fd;
        std::vector<char> buf;
        bool operator==(const Client &other) const
        {
                return fd == other.fd; // or whatever defines equality
        }
        void read();

        Client(Services *services);
        ~Client();
};