#include "../handlers/account_handler.h"
#include "../protocol.h"
#include "../unit_type.h"
#include "../utils.h"

void AccountHandler::SearchWithUsername(Client &client, size_t expectedSize, Services &services)
{
        NetworkingManager *networkManager = services.networkingManager;
        const size_t USERNAME_OFFSET = 3;
        size_t usernameLength = expectedSize - USERNAME_OFFSET;

        if (usernameLength < USERNAME_LENGTH_MIN || usernameLength > USERNAME_LENGTH_MAX)
        {
                return networkManager->sendSearchResponseCode(client, SearchResponseCode::NotFound);
        }
        std::string username(reinterpret_cast<const char *>(&client.buf[USERNAME_OFFSET]), usernameLength);
        Account account;
        bool isFound;
        if (!services.db->getAccountFromUsername(username.c_str(), account, isFound))
        {
                return networkManager->sendSearchResponseCode(client, SearchResponseCode::Error);
        }

        if (!isFound)
        {
                return networkManager->sendSearchResponseCode(client, SearchResponseCode::NotFound);
        }

        std::vector<char> packet;
        packet.push_back(UnitType::searchResponseCode);
        packet.push_back(SearchResponseCode::Found);
        std::vector<char> id = intToBigEndian<std::uint32_t>(account.id);
        packet.insert(packet.end(), id.begin(), id.end());
        packet.insert(packet.end(), account.username.begin(), account.username.end());

        networkManager->secure_send(client, packet);
}

void AccountHandler::SearchWithId(Client &client, size_t expectedSize, Services &services)
{
        NetworkingManager *networkManager = services.networkingManager;
        const size_t ID_OFFSET = 3;
        const size_t ID_LENGTH = 4;
        if (expectedSize < ID_OFFSET + ID_LENGTH)
                return networkManager->sendSearchResponseCode(client, SearchResponseCode::Error);
        uint32_t id = bigEndianToInt<uint32_t>(client.buf, ID_OFFSET);
        Account account;
        bool isFound;
        if (!services.db->getAccountFromId(id, account, isFound))
        {
                return networkManager->sendSearchResponseCode(client, SearchResponseCode::Error);
        }

        if (!isFound)
        {
                return networkManager->sendSearchResponseCode(client, SearchResponseCode::NotFound);
        }

        std::vector<char> packet;
        packet.push_back(UnitType::searchResponseCode);
        packet.push_back(SearchResponseCode::Found);
        std::vector<char> accId = intToBigEndian<std::uint32_t>(account.id);
        packet.insert(packet.end(), accId.begin(), accId.end());
        packet.insert(packet.end(), account.username.begin(), account.username.end());

        networkManager->secure_send(client, packet);
}
