#pragma once
#include <stdint.h>

enum SearchResponseCode : uint8_t
{
        NotFound = 0,
        Found = 1,
        SearchError = 2
};
