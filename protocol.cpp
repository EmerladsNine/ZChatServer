#include "protocol.h"
#include "utils.h"
#include "unit_type.h"
#include <chrono>
#include "database.h"

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

        // TODO You might decrypt here

        handleUnit(client, expectedSize);

        client.buf.erase(client.buf.begin(), client.buf.begin() + expectedSize);
        return true;
}

void Protocol::handleUnit(Client &client, size_t expectedSize)
{
        uint8_t head = static_cast<uint8_t>(client.buf[HEADER_OFFSET]);
        if (head == UnitType::ping)
        {
                // Ping received
                std::vector<char> size = intToBigEndian<std::uint16_t>(1);
                std::vector<char> pongPacket(size.begin(), size.end());
                pongPacket.push_back(1);
                client.networkingManager->safe_send(client, pongPacket);
        }
        else if (head == UnitType::pong)
        {
                // Pong received
        }
        else if (head == UnitType::normalMessage)
        {
                Protocol::handleNormalMessage(client);
        }
        else if (head == UnitType::emailSignIn)
        {
                uint8_t emailSize = static_cast<uint8_t>(client.buf[EMAIL_LENGTH_OFFSET]);
                if (emailSize == 0 || client.buf.size() < EMAIL_OFFSET + emailSize)
                        return; // Invalid just skip the packet.
                std::string email(
                    reinterpret_cast<const char *>(&client.buf[EMAIL_OFFSET]),
                    emailSize);
                std::cout << static_cast<int>(emailSize) << " : " << email << std::endl;
                size_t passwordOffset = EMAIL_OFFSET + emailSize;
                size_t passwordSize = expectedSize - passwordOffset;
                if (passwordSize == 0 || passwordSize > 254)
                        return;
                std::string password(
                    reinterpret_cast<const char *>(&client.buf[passwordOffset]),
                    passwordSize);
                std::cout << passwordSize << " : " << password << std::endl;
        }
        else if (head == UnitType::emailSignUp)
        {
                uint8_t emailLength = static_cast<uint8_t>(client.buf[EMAIL_LENGTH_OFFSET]);
                if (emailLength == 0 || client.buf.size() < EMAIL_OFFSET + emailLength + PASSWORD_LENGTH_SIZE)
                        return; // Todo send error to the user.

                std::string email(
                    reinterpret_cast<const char *>(&client.buf[EMAIL_OFFSET]),
                    emailLength);

                size_t passwordLengthOffset = EMAIL_OFFSET + emailLength;
                size_t passwordLength = static_cast<uint8_t>(client.buf[passwordLengthOffset]);
                size_t passwordOffset = passwordLengthOffset + PASSWORD_LENGTH_SIZE;

                if (passwordLength < 5 || passwordLength > 254 || client.buf.size() < passwordOffset + passwordLength)
                        return; // Todo send error to the user.
                std::string password(
                    reinterpret_cast<const char *>(&client.buf[passwordOffset]),
                    passwordLength);

                size_t usernameOffset = passwordOffset + passwordLength;
                size_t usernameLength = expectedSize - usernameOffset;
                if (usernameLength == 0 || usernameLength > 12)
                        return; // Todo send error to user.
                std::string username(
                    reinterpret_cast<const char *>(&client.buf[usernameOffset]),
                    usernameLength);

                // Creating account.
                Database db;
                bool emailExists;
                if (!db.objExists(db.check_email_exists_stmt, email.c_str(), emailExists))
                        return;
                if (emailExists)
                {
                        std::cout << "email already exist" << std::endl;
                        return;
                }
                bool usernameExists;
                if (!db.objExists(db.check_username_exists_stmt, username.c_str(), usernameExists))
                        return;
                if (usernameExists)
                {
                        std::cout << "username already exist" << std::endl;
                        return;
                }
                db.insertEmailAccount(username.c_str(), email.c_str(), password.c_str());
        }
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