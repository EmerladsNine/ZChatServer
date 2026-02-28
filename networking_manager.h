#pragma once
#include <vector>
#include <stddef.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <cstdio>
#include <iostream>
#include "auth_response_code.h"
#include "search_response_code.h"

class Client;
class Services;

class NetworkingManager
{
private:
        int server_fd;
        fd_set readfds; // The set of clients ready to be read.

public:
        NetworkingManager();
        void safe_send(Client &client, std::vector<char> &buffer);
        void secure_send(Client &client, std::vector<char> &buffer);
        void init(const int PORT);
        void waitForReadableSockets();
        void acceptPendingClients();
        bool isReadable(int client_fd);
        void sendAuthResponseCode(Client &client,AuthResponseCode responseCode);
        void sendSearchResponseCode(Client &client,SearchResponseCode responseCode);
        std::vector<Client> clientsConnected;
        ~NetworkingManager();
};