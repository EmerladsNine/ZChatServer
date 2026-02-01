#pragma once
#include <vector>
#include <unistd.h>
#include <sys/socket.h>
#include <cstdio>
#include <cstdlib>
#include "networking_manager.h"

const uint16_t TIME_STAMP_BYTES = 8;

class Client
{
private:
        NetworkingManager *networkingManager;

public:
        int fd;
        int head;
        int expectedSize;
        std::vector<char> buf;
        bool operator==(const Client &other) const
        {
                return fd == other.fd; // or whatever defines equality
        }
        ssize_t read();
        Client(NetworkingManager *networkingManager);
        ~Client();
};