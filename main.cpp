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

                std::vector<FreeList<Client>::Handle> disconnected;
                auto &clients = services.networkingManager.clientsConnected;
                for (auto handle : clients)
                {
                        Client &client = clients.get(handle);

                        if (!services.networkingManager.isReadable(client.fd))
                                continue;

                        client.read(services);

                        if (!client.isAlive)
                                disconnected.push_back(handle);
                }

                for (auto handle : disconnected)
                {
                        Client &client = clients.get(handle);
                        client.disconnect(services);
                        clients.remove(handle);
                }
        }
}
