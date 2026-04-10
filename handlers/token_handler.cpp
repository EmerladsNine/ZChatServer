#include "token_handler.h"
#include "../utils/endian_codec.h"
#include "../unit_type.h"
#include "../protocol.h"

TokenHandler::TokenHandler()
{
}

TokenHandler::~TokenHandler()
{
}

void TokenHandler::useAccessToken(Client &client, expected_size expectedSize, Services &services)
{
        const size_t SESSION_ID_OFFSET = HEADER_OFFSET + HEAD_SIZE;
        const size_t SESSION_ID_SIZE = sizeof(sessionIdType);
        const size_t ACCESS_TOKEN_OFFSET = SESSION_ID_OFFSET + SESSION_ID_SIZE;
        const size_t ACCESS_TOKEN_SIZE = 32;
        if (expectedSize < ACCESS_TOKEN_OFFSET + ACCESS_TOKEN_SIZE)
                return services.networkingManager.sendSessionStateResponseCode(client, SessionStateResponseCode::AuthenticationFailure);
        sessionIdType sessionId = bigEndianToInt<sessionIdType>(client.buf, SESSION_ID_OFFSET);
        std::string accessToken(client.buf.begin() + ACCESS_TOKEN_OFFSET, client.buf.begin() + ACCESS_TOKEN_OFFSET + ACCESS_TOKEN_SIZE);
        Session session;
        bool isFound;
        if (!services.db.getSessionFromId(sessionId, session, isFound))
                return services.networkingManager.sendSessionStateResponseCode(client, SessionStateResponseCode::AuthenticationFailure);
        if (!isFound)
                return services.networkingManager.sendSessionStateResponseCode(client, SessionStateResponseCode::AuthenticationFailure);
        if (!session.isAccessTokenActive(30))
                return services.networkingManager.sendSessionStateResponseCode(client, SessionStateResponseCode::AccessTokenExpired);
        if (!services.hashManager.sha256Hash.Verify(accessToken, session.accessTokenHash))
                return services.networkingManager.sendSessionStateResponseCode(client, SessionStateResponseCode::AuthenticationFailure);
        client.authenticate(services, session);
        return services.networkingManager.sendSessionStateResponseCode(client, SessionStateResponseCode::SessionAuthenticationSuccess);
}

void TokenHandler::useRefreshToken(Client &client, expected_size expectedSize, Services &services)
{
        const size_t SESSION_ID_OFFSET = HEADER_OFFSET + HEAD_SIZE;
        const size_t SESSION_ID_SIZE = sizeof(sessionIdType);
        const size_t REFRESH_TOKEN_OFFSET = SESSION_ID_OFFSET + SESSION_ID_SIZE;
        const size_t REFRESH_TOKEN_SIZE = 64;
        if (expectedSize < REFRESH_TOKEN_OFFSET + REFRESH_TOKEN_SIZE)
                return services.networkingManager.sendSessionStateResponseCode(client, SessionStateResponseCode::AuthenticationFailure);
        sessionIdType sessionId = bigEndianToInt<sessionIdType>(client.buf, SESSION_ID_OFFSET);
        std::string refreshToken(client.buf.begin() + REFRESH_TOKEN_OFFSET, client.buf.begin() + REFRESH_TOKEN_OFFSET + REFRESH_TOKEN_SIZE);
        Session session;
        if (!client.inSession || client.session.sessionId != sessionId)
        {
                bool isFound;
                if (!services.db.getSessionFromId(sessionId, session, isFound))
                        return services.networkingManager.sendSessionStateResponseCode(client, SessionStateResponseCode::AuthenticationFailure);
                if (!isFound)
                        return services.networkingManager.sendSessionStateResponseCode(client, SessionStateResponseCode::AuthenticationFailure);
        }
        else
        {
                session = client.session;
        }
        bool isEqual;
        if (!services.hashManager.argonHash.verifyPassword(refreshToken.data(), refreshToken.size(), session.refreshTokenHash.data(), isEqual))
                return services.networkingManager.sendSessionStateResponseCode(client, SessionStateResponseCode::AuthenticationFailure);
        if (!isEqual)
                return services.networkingManager.sendSessionStateResponseCode(client, SessionStateResponseCode::AuthenticationFailure);
        if (!session.isRefreshTokenActive(30))
                return services.networkingManager.sendSessionStateResponseCode(client, SessionStateResponseCode::RefreshTokenExpired);
        // Generate new session tokens
        std::vector<char> newAccessToken;
        std::vector<char> newRefreshToken;
        std::string newAccessTokenHash;
        std::string newRefreshTokenHash;
        if (!services.tokenGen.generateSession(services, newAccessToken, newRefreshToken, newAccessTokenHash, newRefreshTokenHash))
                return services.networkingManager.sendSessionStateResponseCode(client, SessionStateResponseCode::AuthenticationFailure);
        if (!services.db.updateSession(sessionId, newAccessTokenHash, newRefreshTokenHash.data()))
                return services.networkingManager.sendSessionStateResponseCode(client, SessionStateResponseCode::AuthenticationFailure);
        bool isFound;
        if (!services.db.getSessionFromId(sessionId, session, isFound))
                return services.networkingManager.sendSessionStateResponseCode(client, SessionStateResponseCode::AuthenticationFailure);
        if (!isFound)
                return services.networkingManager.sendSessionStateResponseCode(client, SessionStateResponseCode::AuthenticationFailure);
        client.authenticate(services, session);
        // Send Session.
        std::vector<char> packet;
        packet.push_back(UnitType::sessionStateResponseCode);
        packet.push_back(SessionStateResponseCode::RefreshSuccess);
        packet.insert(packet.end(), newAccessToken.begin(), newAccessToken.end());
        packet.insert(packet.end(), newRefreshToken.begin(), newRefreshToken.end());
        services.networkingManager.secure_send(client, packet);
}