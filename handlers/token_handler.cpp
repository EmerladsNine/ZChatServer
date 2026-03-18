#include "token_handler.h"
#include "../utils.h"

TokenHandler::TokenHandler()
{
}

TokenHandler::~TokenHandler()
{
}

void TokenHandler::useAccessToken(Client &client, size_t expectedSize, Services &services)
{
        const size_t SESSION_ID_OFFSET = 3;
        const size_t SESSION_ID_SIZE = 4;
        const size_t ACCESS_TOKEN_OFFSET = SESSION_ID_OFFSET + SESSION_ID_SIZE;
        const size_t ACCESS_TOKEN_SIZE = 32;
        if (expectedSize < ACCESS_TOKEN_OFFSET + ACCESS_TOKEN_SIZE)
                return services.networkingManager.sendSessionStateResponseCode(client, SessionStateResponseCode::AuthenticationFailure);
        uint32_t sessionId = bigEndianToInt<uint32_t>(client.buf, SESSION_ID_OFFSET);
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