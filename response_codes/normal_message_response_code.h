#pragma once
#include <stdint.h>

enum NormalMessageResponseCode : uint8_t
{
        NormalMessageResponseFailure = 0,
        NormalMessageResponseSuccess = 1,
        OutdatedSessionListVersion = 2,
        UserNotFound = 3
};