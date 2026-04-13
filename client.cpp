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
        if (inSession)
        {
                std::cout << "LOG OUT : " << session.sessionId << std::endl;
                services.networkingManager.onlineUsers.erase(session.sessionId);
                inSession = false;
                isSessionValid = false;
        }
        services.networkingManager.onlineUsers[sessionToAdd.sessionId] = handle;
        session = sessionToAdd;
        inSession = true;
        if (session.isAccessTokenActive(30))
        {
                isSessionValid = true;
                std::vector<Message> messages;
                if (services.db.getMessagesForSession(session.sessionId, messages))
                {
                        for (Message &msg : messages)
                        {
                                services.networkingManager.secure_send(*this, msg.message);
                        }
                        if (!messages.empty())
                        {
                                int64_t lastTimestamp = messages.back().timestamp;
                                services.db.deleteMessagesBefore(session.sessionId, lastTimestamp);
                        }
                }
        }
        std::cout << "Authenticated : " << sessionToAdd.sessionId << std::endl;
}

void Client::disconnect(Services &services)
{
        if (inSession)
        {
                inSession = false;
                isSessionValid = false;
                services.networkingManager.onlineUsers.erase(session.sessionId);
        }
}