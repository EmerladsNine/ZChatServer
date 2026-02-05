#include "client.h"
#include <algorithm>
#include <vector>
#include "networking_manager.h"

int main()
{
        const int PORT = 9999;
        Database db;
        NetworkingManager networkingManager(&db);
        networkingManager.init(PORT);
        while (true)
        {
                networkingManager.waitForReadableSockets();
                networkingManager.acceptPendingClients(); // Checks if there is any new client that wants to connect.

                std::vector<Client> disconnected;
                for (Client &client : networkingManager.clientsConnected)
                {
                        if (!networkingManager.isReadable(client.fd))
                                continue;

                        client.read();

                        if (!client.isAlive)
                                disconnected.push_back(client);
                }

                for (Client &client : disconnected)
                {
                        networkingManager.clientsConnected.erase(
                            std::remove(networkingManager.clientsConnected.begin(), networkingManager.clientsConnected.end(), client),
                            networkingManager.clientsConnected.end());
                }
        }
}
