#include "protocol.h"
#include "utils/endian_codec.h"
#include "unit_type.h"
#include <chrono>
#include "database_managment/database.h"
#include "handlers/account_handler.h"
#include "handlers/token_handler.h"
#include "handlers/session_list_handler.h"
#include "handlers/fcm_handler.h"
#include "response_codes/normal_message_response_code.h"

using namespace std::chrono;

Protocol::Protocol()
{
}

Protocol::~Protocol()
{
}

bool getExpectedSize(Client &client, expected_size &out)
{
        if (client.buf.size() < EXPECTED_SIZE_BYTES)
                return false;

        expected_size size = bigEndianToInt<expected_size>(client.buf, 0);
        if (size == 0)
                return false;
        out = size + EXPECTED_SIZE_BYTES;
        return true;
}

bool Protocol::parseUnit(Client &client, Services &services)
{
        expected_size expectedSize;
        if (!getExpectedSize(client, expectedSize))
                return false;
        if (client.buf.size() < expectedSize)
                return false;

        // Todo You might decrypt here

        handleUnit(client, expectedSize, services);

        client.buf.erase(client.buf.begin(), client.buf.begin() + expectedSize);
        return true;
}

void Protocol::handleUnit(Client &client, expected_size expectedSize, Services &services)
{
        if (client.inSession && !client.session.isAccessTokenActive(30))
                client.isSessionValid = false;
        uint8_t head = static_cast<uint8_t>(client.buf[HEADER_OFFSET]);
        if (head == UnitType::ping)
        {
                // Ping received
                std::vector<char> pongPacket;
                pongPacket.push_back(1);
                services.networkingManager.secure_send(client, pongPacket);
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
        else if (head == UnitType::useAccessToken)
        {
                TokenHandler::useAccessToken(client, expectedSize, services);
        }
        else if (head == UnitType::useRefreshToken)
        {
                TokenHandler::useRefreshToken(client, expectedSize, services);
        }
        else if (head == UnitType::requestSessionsList)
        {
                SessionListHandler::handleSessionListRequest(client, expectedSize, services);
        }
        else if (head == UnitType::syncFcmToken)
        {
                FcmHandler::handleFcmToken(client, expectedSize, services);
        }
}

void Protocol::handleNormalMessage(Client &client, expected_size expectedSize, Services &services)
{
        if (!client.inSession || !client.isSessionValid)
                return services.networkingManager.sendSessionStateResponseCode(client, SessionStateResponseCode::NotAuthenticated);

        std::vector<char> senderPacket;
        senderPacket.push_back(UnitType::normalMessageResponseCode);
        size_t currentOffset = HEADER_OFFSET + HEAD_SIZE;
        while (currentOffset < expectedSize)
        {
                userIdType receiverId = bigEndianToInt<userIdType>(client.buf, currentOffset);
                currentOffset += sizeof(userIdType);
                std::vector<char> receiverIdVec = intToBigEndian<userIdType>(receiverId);
                senderPacket.insert(senderPacket.end(), receiverIdVec.begin(), receiverIdVec.end());
                sessionListVersionType version = bigEndianToInt<sessionListVersionType>(client.buf, currentOffset);
                currentOffset += sizeof(sessionListVersionType);
                SessionList sessionList(0);
                auto it = services.networkingManager.sessionsListCache.find(receiverId);
                if (it != services.networkingManager.sessionsListCache.end())
                {
                        sessionList = it->second;
                }
                else
                {

                        Account account;
                        bool isFound;
                        if (!services.db.getAccountFromId(receiverId, account, isFound))
                        {
                                senderPacket.push_back(NormalMessageResponseCode::NormalMessageResponseFailure);
                                continue;
                        }
                        if (!isFound)
                        {
                                senderPacket.push_back(NormalMessageResponseCode::UserNotFound);
                                continue;
                        }
                        sessionList.version = account.sessionListVersion;
                        std::vector<Session> sessions;
                        services.db.getSessionsFromUserId(receiverId, sessions);
                        for (Session &session : sessions)
                        {
                                if (!session.isRefreshTokenActive())
                                        continue;
                                sessionList.sessions.push_back(session.sessionId);
                        }
                        services.networkingManager.sessionsListCache[receiverId] = sessionList;
                }

                if (sessionList.version != version)
                {
                        senderPacket.push_back(NormalMessageResponseCode::OutdatedSessionListVersion);
                        std::vector<char> sessionListVersionVec = intToBigEndian<sessionListVersionType>(sessionList.version);
                        senderPacket.insert(senderPacket.end(), sessionListVersionVec.begin(), sessionListVersionVec.end());
                        senderPacket.push_back(sessionList.sessions.size());
                        for (sessionIdType sessionId : sessionList.sessions)
                        {
                                std::vector<char> sessionIdVec = intToBigEndian<sessionIdType>(sessionId);
                                senderPacket.insert(senderPacket.end(), sessionIdVec.begin(), sessionIdVec.end());
                        }
                        continue;
                }

                size_t sessionsCount = bigEndianToInt<char>(client.buf, currentOffset);
                currentOffset += sizeof(char);
                for (int i = 0; i < sessionsCount; i++)
                {
                        sessionIdType sessionId = bigEndianToInt<sessionIdType>(client.buf, currentOffset);
                        currentOffset += sizeof(sessionIdType);
                        expected_size messageLength = bigEndianToInt<expected_size>(client.buf, currentOffset);
                        currentOffset += sizeof(expected_size);
                        std::vector<char> messageBody(client.buf.begin() + currentOffset, client.buf.begin() + currentOffset + messageLength);
                        currentOffset += messageLength;
                        std::vector<char> senderId = intToBigEndian<userIdType>(client.session.userid);
                        std::int64_t timestamp = duration_cast<microseconds>(system_clock::now().time_since_epoch()).count();
                        std::vector<char> timestampVec = intToBigEndian<std::int64_t>(timestamp);
                        std::vector<char> packet;
                        packet.reserve(HEAD_SIZE + timestampVec.size() + messageBody.size());
                        packet.push_back(UnitType::normalMessage);
                        packet.insert(packet.end(), senderId.begin(), senderId.end());
                        packet.insert(packet.end(), timestampVec.begin(), timestampVec.end());
                        packet.insert(packet.end(), messageBody.begin(), messageBody.end());

                        if (std::find(sessionList.sessions.begin(), sessionList.sessions.end(), sessionId) == sessionList.sessions.end())
                                continue;

                        auto it = services.networkingManager.onlineUsers.find(sessionId);
                        if (it != services.networkingManager.onlineUsers.end())
                        {
                                std::cout << "Sending" << std::endl;
                                Client &receiverClient = services.networkingManager.clientsConnected.get(it->second);
                                services.networkingManager.secure_send(receiverClient, packet);
                        }
                        else
                        {
                                std::string message(packet.begin(), packet.end());
                                services.db.insertMessage(client.session.userid, sessionId, message, timestamp);
                                Session session;
                                bool isFound;
                                if (services.db.getSessionFromId(sessionId, session, isFound) && isFound && !session.fcmToken.empty() && services.curlManager.valid)
                                {
                                        std::string jsonBody = "{"
                                                               "\"device_token\": \"" +
                                                               session.fcmToken + "\","
                                                                                  "\"title\": \"ZChat Notification\","
                                                                                  "\"body\": \"you got a new message!\""
                                                                                  "}";
                                        std::string out;
                                        curl_easy_setopt(services.curlManager.FcmCurl, CURLOPT_WRITEDATA, &out);
                                        curl_easy_setopt(services.curlManager.FcmCurl, CURLOPT_POSTFIELDS, jsonBody.c_str());
                                        CURLcode res = curl_easy_perform(services.curlManager.FcmCurl);
                                }
                        }
                }
                senderPacket.push_back(NormalMessageResponseCode::NormalMessageResponseSuccess);
        }
        services.networkingManager.secure_send(client, senderPacket);
}