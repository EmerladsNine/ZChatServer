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
        bool isAlive = true;
        bool canRename;
        int fd;
        std::vector<char> buf;
        bool operator==(const Client &other) const
        {
                return fd == other.fd; // or whatever defines equality
        }
        void read(Services &services);

        Client();
        ~Client();
};