#pragma once
#include <string>
#include <cstdint>

class Session
{
private:
public:
        std::string accessTokenHash;
        uint8_t userid;
        Session();
        ~Session();
};
