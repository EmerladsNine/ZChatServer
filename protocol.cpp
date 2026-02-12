#include "protocol.h"
#include "utils.h"
#include "unit_type.h"
#include <chrono>
#include "database.h"
#include "auth/email_auth.h"
#include "auth/google_auth.h"

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

        size_t size = readUint16FromBuffer(client.buf, 0);
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
        NetworkingManager *networkManager = services.networkingManager;
        uint8_t head = static_cast<uint8_t>(client.buf[HEADER_OFFSET]);
        if (head == UnitType::ping)
        {
                // Ping received
                std::vector<char> size = intToBigEndian<std::uint16_t>(1);
                std::vector<char> pongPacket(size.begin(), size.end());
                pongPacket.push_back(1);
                services.networkingManager->safe_send(client, pongPacket);
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
                EmailAuth::SignIn(client, expectedSize, services);
        }
        else if (head == UnitType::emailSignUp)
        {
                EmailAuth::SignUp(client, expectedSize, services);
        }
        else if (head == UnitType::googleSignIn)
        {
                GoogleAuth::SignIn(client, expectedSize, services);
        }
        else if (head == UnitType::googleSignUp)
        {
                GoogleAuth::SignUp(client, expectedSize, services);
        }
}

void Protocol::handleNormalMessage(Client &client, size_t expectedSize, Services &services)
{
        std::vector<char> packet(client.buf.begin(), client.buf.begin() + expectedSize);

        // Update size in new packet
        size_t newPacketSize = (expectedSize - EXPECTED_SIZE_BYTES) + TIME_STAMP_BYTES;
        std::vector<char> size = intToBigEndian<std::uint16_t>(newPacketSize);
        std::copy(size.begin(), size.end(), packet.begin());

        // Insertion of timestamp
        std::vector<char> timeStamp = intToBigEndian<std::int64_t>(duration_cast<microseconds>(system_clock::now().time_since_epoch()).count());
        packet.insert(packet.begin() + TIME_STAMP_OFFSET, timeStamp.begin(), timeStamp.end());

        // Broadcasting
        for (Client &other : services.networkingManager->clientsConnected)
        {
                if (other.fd != client.fd)
                {
                        services.networkingManager->safe_send(other, packet);
                }
        }
}