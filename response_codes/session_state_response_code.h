#pragma once
#include <stdint.h>

enum SessionStateResponseCode : uint8_t
{
        NotAuthenticated = 0,
        AuthenticationFailure = 1,
        AccessTokenExpired = 2,
        SessionAuthenticationSuccess = 3,
        RefreshTokenExpired = 4,
        RefreshSuccess = 5
};
