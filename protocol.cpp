#include "protocol.h"
#include "utils.h"
#include "unit_type.h"
#include <chrono>
#include "database_managment/database.h"
#include "handlers/account_handler.h"
using namespace std::chrono;

Protocol::Protocol()
{
}

Protocol::~Protocol()
{
}

bool getExpectedSize(Client &client, size_t &out)
{
        if (client.buf.size() < EXPECTED_SIZE_BYTES)
                return false;

        size_t size = bigEndianToInt<uint16_t>(client.buf, 0);
        if (size == 0)
                return false;
        out = size + EXPECTED_SIZE_BYTES;
        return true;
}

bool Protocol::parseUnit(Client &client, Services &services)
{
        size_t expectedSize;
        if (!getExpectedSize(client, expectedSize))
                return false;
        if (client.buf.size() < expectedSize)
                return false;

        // Todo You might decrypt here

        handleUnit(client, expectedSize, services);

        client.buf.erase(client.buf.begin(), client.buf.begin() + expectedSize);
        return true;
}

void Protocol::handleUnit(Client &client, size_t expectedSize, Services &services)
{
        uint8_t head = static_cast<uint8_t>(client.buf[HEADER_OFFSET]);
        if (head == UnitType::ping)
        {
                // Ping received
                std::vector<char> size = intToBigEndian<std::uint16_t>(1);
                std::vector<char> pongPacket(size.begin(), size.end());
                pongPacket.push_back(1);
                services.networkingManager.safe_send(client, pongPacket);
        }
        else if (head == UnitType::pong)
        {
                // Pong received
        }
        else if (head == UnitType::normalMessage)
        {
                Protocol::handleNormalMessage(client, expectedSize, services);
        }
        else if (head == UnitType::emailSignIn)
        {
                AccountHandler::EmailSignIn(client, expectedSize, services);
        }
        else if (head == UnitType::emailSignUp)
        {
                AccountHandler::EmailSignUp(client, expectedSize, services);
        }
        else if (head == UnitType::googleSignIn)
        {
                AccountHandler::GoogleSignIn(client, expectedSize, services);
        }
        else if (head == UnitType::googleSignUp)
        {
                AccountHandler::GoogleSignUp(client, expectedSize, services);
        }
        else if (head == UnitType::searchWithUsername)
        {
                AccountHandler::SearchWithUsername(client, expectedSize, services);
        }
        else if (head == UnitType::searchWithId)
        {
                AccountHandler::SearchWithId(client, expectedSize, services);
        }
}

void Protocol::handleNormalMessage(Client &client, size_t expectedSize, Services &services)
{

        const size_t RECEIVER_ID_OFFSET = HEADER_OFFSET + HEAD_SIZE;
        const size_t RECEIVER_ID_SIZE = 4;
        const size_t MESSAGE_BODY_OFFSET = RECEIVER_ID_OFFSET + RECEIVER_ID_SIZE;

        if (!client.isAuthenticated)
                return services.networkingManager.sendNotAuthenticated(client);
        uint32_t receiverId = bigEndianToInt<std::uint32_t>(client.buf, RECEIVER_ID_OFFSET);
        std::cout << receiverId << std::endl;
        std::vector<char> messageBody(client.buf.begin() + MESSAGE_BODY_OFFSET, client.buf.begin() + expectedSize);
        std::vector<char> senderId = intToBigEndian<std::uint32_t>(client.session.userid);
        std::vector<char> timeStamp = intToBigEndian<std::int64_t>(duration_cast<microseconds>(system_clock::now().time_since_epoch()).count());
        std::vector<char> packet;
        packet.reserve(HEAD_SIZE + timeStamp.size() + messageBody.size());
        packet.push_back(UnitType::normalMessage);
        packet.insert(packet.end(), senderId.begin(), senderId.end());
        packet.insert(packet.end(), timeStamp.begin(), timeStamp.end());
        packet.insert(packet.end(), messageBody.begin(), messageBody.end());

        // send ok to the sender client.
        std::vector<char> okPacket;
        okPacket.push_back(UnitType::normalMessageResponseCode);
        services.networkingManager.secure_send(client, okPacket);

        // Find receiver and send
        auto it = services.networkingManager.onlineUsers.find(receiverId);
        if (it != services.networkingManager.onlineUsers.end())
        {
                std::cout << "Sending" << std::endl;
                Client *receiverClient = it->second;
                services.networkingManager.secure_send(*receiverClient, packet);
        }
}