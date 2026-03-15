#include "client.h"
#include "protocol.h"
#include <chrono>

using namespace std::chrono;

Client::Client()
{
}

Client::~Client()
{
}

void Client::read(Services &services)
{
        char temp[4096];
        ssize_t n = recv(fd, temp, sizeof(temp), 0);

        if (n <= 0)
        {
                // Client Disconnected.
                close(fd);
                isAlive = false;
                return;
        }

        buf.insert(buf.end(), temp, temp + n);

        while (buf.size() > 0)
        {
                if (!Protocol::parseUnit(*this, services))
                        return;
        }
}

void Client::authenticate(Services &services, uint32_t id, std::string &accessTokenHash)
{
        if (isAuthenticated)
        {
                std::cout << "LOG OUT : " << session.userid << std::endl;
                services.networkingManager.onlineUsers.erase(session.userid);
                isAuthenticated = false;
        }
        services.networkingManager.onlineUsers[id] = this;
        isAuthenticated = true;
        session.userid = id;
        session.accessTokenHash = accessTokenHash;
        std::cout << "Authenticated : " << id << std::endl;
}

void Client::disconnect(Services &services)
{
        if (isAuthenticated)
        {
                services.networkingManager.onlineUsers.erase(session.userid);
                isAuthenticated = false;
        }
}