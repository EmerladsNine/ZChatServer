#include "client.h"
#include <algorithm>
#include <vector>
#include "services.h"

int main()
{
        const int PORT = 9999;
        Services services;

        services.networkingManager.init(PORT);
        while (true)
        {
                services.networkingManager.waitForReadableSockets();
                services.networkingManager.acceptPendingClients(); // Checks if there is any new client that wants to connect.

                std::vector<Client> disconnected;
                for (Client &client : services.networkingManager.clientsConnected)
                {
                        if (!services.networkingManager.isReadable(client.fd))
                                continue;

                        client.read(services);

                        if (!client.isAlive)
                                disconnected.push_back(client);
                }

                for (Client &client : disconnected)
                {
                        services.networkingManager.clientsConnected.erase(
                            std::remove(services.networkingManager.clientsConnected.begin(), services.networkingManager.clientsConnected.end(), client),
                            services.networkingManager.clientsConnected.end());
                }
        }
}
