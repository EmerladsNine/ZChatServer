#include "session.h"
#include <ctime>

Session::Session()
{
}

Session::~Session()
{
}

bool Session::isAccessTokenActive(int64_t bufferSeconds)
{
        return accessExpiry > (std::time(nullptr) + bufferSeconds);
}

bool Session::isRefreshTokenActive(int64_t bufferSeconds)
{
        return refreshExpiry > (std::time(nullptr) + bufferSeconds);
}