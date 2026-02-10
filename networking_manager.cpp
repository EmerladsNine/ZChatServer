#include "networking_manager.h"
#include "client.h"
#include "utils.h"
#include "unit_type.h"

NetworkingManager::NetworkingManager(Database *db) : db(db) {}

NetworkingManager::~NetworkingManager() {}

bool NetworkingManager::isReadable(int client_fd)
{
        return FD_ISSET(client_fd, &readfds);
}

void NetworkingManager::safe_send(Client &client, std::vector<char> &buffer)
{
        size_t totalSent = 0;
        while (totalSent < buffer.size())
        {
                size_t n = send(client.fd, buffer.data(), buffer.size(), 0);
                if (n <= 0)
                {
                        // Todo Handle this
                        perror("send");
                        exit(1);
                }
                totalSent += n;
        }
}

void NetworkingManager::secure_send(Client &client, std::vector<char> &buffer)
{
        // You encrypt first then get the size and insert it.
        size_t size = buffer.size();
        std::vector<char> sizeBytes = intToBigEndian<std::uint16_t>(size);
        buffer.insert(buffer.begin(), sizeBytes.begin(), sizeBytes.end());
        safe_send(client, buffer);
}

void NetworkingManager::init(const int PORT)
{
        server_fd = socket(AF_INET, SOCK_STREAM, 0);
        if (server_fd < 0)
        {
                perror("socket");
                exit(1);
        }

        int opt = 1;
        setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));

        sockaddr_in addr;
        addr.sin_family = AF_INET;
        addr.sin_addr.s_addr = INADDR_ANY;
        addr.sin_port = htons(PORT);

        if (bind(server_fd, (sockaddr *)&addr, sizeof(addr)) < 0)
        {
                perror("bind");
                exit(1);
        }

        if (listen(server_fd, 10) < 0)
        {
                perror("listen");
                exit(1);
        }
        std::cout << "Server Listening on port " << PORT << "\n";
}

void NetworkingManager::waitForReadableSockets()
{
        FD_ZERO(&readfds); // Clear all bits
        FD_SET(server_fd, &readfds);

        // get max fd
        int max_fd = server_fd;
        for (Client &client : clientsConnected)
        {
                FD_SET(client.fd, &readfds);
                max_fd = std::max(max_fd, client.fd);
        }

        // waits until at least one socket is readable and removes every not
        // readable socket from readfds
        if (select(max_fd + 1, &readfds, nullptr, nullptr, nullptr) < 0)
        {
                // Todo Handle this
                perror("select");
                exit(1);
        }
}

void NetworkingManager::acceptPendingClients()
{
        // if server socket is readable then there is a new client connects
        if (FD_ISSET(server_fd, &readfds))
        {
                int client_fd = accept(server_fd, nullptr, nullptr);
                if (client_fd >= 0)
                {
                        std::cout << "New Client Connected : " << client_fd << std::endl;
                        Client client(this, db);
                        client.fd = client_fd;
                        clientsConnected.push_back(client);
                }
        }
}

void NetworkingManager::sendResponseCode(Client &client, ResponseCode responseCode)
{
        std::vector<char> packet;
        packet.push_back(UnitType::responseCode);
        packet.push_back(responseCode);
        secure_send(client, packet);
}