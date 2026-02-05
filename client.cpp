#include "client.h"
#include "protocol.h"
#include <chrono>

using namespace std::chrono;

Client::Client(NetworkingManager *networkingManager, Database *db)
    : networkingManager(networkingManager), db(db)
{
        isAlive = true;
}

Client::~Client()
{
}

void Client::read()
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
                if (!Protocol::parseUnit(*this))
                        return;
        }
}