#include "session_list_handler.h"
#include "../protocol.h"
#include "../utils/endian_codec.h"
#include "../unit_type.h"
#include "../response_codes/session_list_response_code.h"

void SessionListHandler::handleSessionListRequest(Client &client, expected_size expectedSize, Services &services)
{
        const size_t USER_IDS_COUNT_OFFSET = HEADER_OFFSET + HEAD_SIZE;
        const size_t USER_IDS_COUNT_LENGTH = 1;
        if (expectedSize < USER_IDS_COUNT_OFFSET + USER_IDS_COUNT_LENGTH)
                return services.networkingManager.sendSessionListResponseCode(client, SessionListResponseCode::Error);
        size_t UserIdsCount = client.buf[USER_IDS_COUNT_OFFSET];
        const size_t USER_ID_LENGTH = sizeof(userIdType);
        if (expectedSize < USER_IDS_COUNT_OFFSET + USER_IDS_COUNT_LENGTH + UserIdsCount * USER_ID_LENGTH)
                return services.networkingManager.sendSessionListResponseCode(client, SessionListResponseCode::Error);

        std::vector<char> packet;
        packet.push_back(UnitType::requestSessionListResponseCode);
        packet.push_back(SessionListResponseCode::Success);
        for (int i = 0; i < UserIdsCount; i++)
        {
                userIdType userId = bigEndianToInt<userIdType>(client.buf, USER_IDS_COUNT_OFFSET + USER_IDS_COUNT_LENGTH + i * USER_ID_LENGTH);
                std::vector<char> userIdVec(client.buf.begin() + USER_IDS_COUNT_OFFSET + USER_IDS_COUNT_LENGTH + i * USER_ID_LENGTH, client.buf.begin() + USER_IDS_COUNT_OFFSET + USER_IDS_COUNT_LENGTH + (i + 1) * USER_ID_LENGTH);
                packet.insert(packet.end(), userIdVec.begin(), userIdVec.end());
                SessionList sessionList(0);
                if (!services.networkingManager.sessionsListCache.contains(userId))
                {
                        Account account;
                        bool isFound;
                        if (!services.db.getAccountFromId(userId, account, isFound) || !isFound)
                                return services.networkingManager.sendSessionListResponseCode(client, SessionListResponseCode::Error);
                        sessionList.version = account.sessionListVersion;
                        std::vector<Session> sessions;
                        services.db.getSessionsFromUserId(userId, sessions);
                        for (Session &session : sessions)
                        {
                                if (!session.isRefreshTokenActive())
                                        continue;
                                sessionList.sessions.push_back(session.sessionId);
                        }
                        services.networkingManager.sessionsListCache[userId] = sessionList;
                }
                else
                {
                        sessionList = services.networkingManager.sessionsListCache[userId];
                }
                std::vector<char> versionVec = intToBigEndian<sessionListVersionType>(sessionList.version);
                packet.insert(packet.end(), versionVec.begin(), versionVec.end());
                size_t sessionsSize = sessionList.sessions.size();
                std::vector<char> sessionsSizeVec = intToBigEndian<size_t>(sessionsSize);
                packet.insert(packet.end(), sessionsSizeVec.begin(), sessionsSizeVec.end());
                for (size_t i = 0; i < sessionsSize; i++)
                {
                        std::vector<char> sessionVec = intToBigEndian<sessionIdType>(sessionList.sessions[i]);
                        packet.insert(packet.end(), sessionVec.begin(), sessionVec.end());
                }
                services.networkingManager.secure_send(client, packet);
        }
}