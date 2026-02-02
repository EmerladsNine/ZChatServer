#include "protocol.h"
#include "utils.h"
#include <chrono>

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

bool Protocol::parseUnit(Client &client)
{
        size_t expectedSize;
        if (!getExpectedSize(client, expectedSize))
                return false;

        if (client.buf.size() < expectedSize)
                return false;

        uint8_t head = static_cast<uint8_t>(client.buf[HEADER_OFFSET]);

        if (head == 0)
        {
                // Ping received
                std::vector<char> size = intToBigEndian<std::uint16_t>(1);
                std::vector<char> pongPacket(size.begin(), size.end());
                pongPacket.push_back(1);
                client.networkingManager->safe_send(client, pongPacket);
        }
        else if (head == 1)
        {
                // Pong received
        }
        else if (head == 2)
        {
                Protocol::handleNormalMessage(client);
        }

        client.buf.erase(client.buf.begin(), client.buf.begin() + expectedSize);
        return true;
}

void Protocol::handleNormalMessage(Client &client)
{

        size_t expectedSize;
        if (!getExpectedSize(client, expectedSize))
                return;

        std::vector<char> packet(client.buf.begin(), client.buf.begin() + expectedSize);

        // Update size in new packet
        size_t newPacketSize = (expectedSize - EXPECTED_SIZE_BYTES) + TIME_STAMP_BYTES;
        std::vector<char> size = intToBigEndian<std::uint16_t>(newPacketSize);
        std::copy(size.begin(), size.end(), packet.begin());

        // Insertion of timestamp
        std::vector<char> timeStamp = intToBigEndian<std::int64_t>(duration_cast<microseconds>(system_clock::now().time_since_epoch()).count());
        packet.insert(packet.begin() + TIME_STAMP_OFFSET, timeStamp.begin(), timeStamp.end());

        // Broadcasting
        for (Client &other : client.networkingManager->clientsConnected)
        {
                if (other.fd != client.fd)
                {
                        client.networkingManager->safe_send(other, packet);
                }
        }
}