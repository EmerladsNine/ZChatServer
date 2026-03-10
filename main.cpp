#include "client.h"
#include <algorithm>
#include <vector>
#include "services.h"

int main()
{
        const int PORT = 9999;
        Database db;
        NetworkingManager networkingManager;
        ArgonHash argonHash;

        Services services;
        services.db = &db;
        services.networkingManager = &networkingManager;
        services.argonHash = &argonHash;

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

                        client.read(services);

                        if (!client.isAlive)
                                disconnected.push_back(client);
                }

                for (Client &client : disconnected)
                {
                        client.disconnect(services);
                        services.networkingManager.clientsConnected.erase(
                            std::remove(services.networkingManager.clientsConnected.begin(), services.networkingManager.clientsConnected.end(), client),
                            services.networkingManager.clientsConnected.end());
                }
        }
}
