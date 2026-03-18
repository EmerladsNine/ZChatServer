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

void Client::authenticate(Services &services, Session sessionToAdd)
{
        if (isAuthenticated)
        {
                std::cout << "LOG OUT : " << sessionToAdd.userid << std::endl;
                services.networkingManager.onlineUsers.erase(sessionToAdd.userid);
                isAuthenticated = false;
        }
        services.networkingManager.onlineUsers[sessionToAdd.userid] = this;
        isAuthenticated = true;
        session = sessionToAdd;
        std::cout << "Authenticated : " << sessionToAdd.userid << std::endl;
}

void Client::disconnect(Services &services)
{
        if (isAuthenticated)
        {
                services.networkingManager.onlineUsers.erase(session.userid);
                isAuthenticated = false;
        }
}