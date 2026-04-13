#pragma once
#include <string>
#include <cstdint>
#include "account.h"

class Session
{
private:
public:
        std::string refreshTokenHash;
        std::string accessTokenHash;
        userIdType userid;
        sessionIdType sessionId;
        int64_t accessExpiry;
        int64_t refreshExpiry;
        Session();
        bool isAccessTokenActive(int64_t bufferSeconds = 0);
        bool isRefreshTokenActive(int64_t bufferSeconds = 0);
        ~Session();
};
