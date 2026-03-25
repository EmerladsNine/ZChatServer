#pragma once
#include <vector>
#include <unistd.h>
#include <sys/socket.h>
#include <cstdio>
#include <cstdlib>
#include "services.h"
#include "database_managment/data/session.h"
#include "utils/free_list.h"

class Client
{
private:
public:
        bool isAlive = true;
        bool canRename;
        Session session;
        bool isSessionValid = false;
        bool inSession = false;
        int fd;
        FreeList<Client>::Handle handle;
        std::vector<char> buf;
        bool operator==(const Client &other) const
        {
                return fd == other.fd;
        }
        void read(Services &services);
        void authenticate(Services &services, Session sessionToAdd);
        void disconnect(Services &services);
        Client();
        ~Client();
};