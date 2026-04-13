#pragma once
#include <string>
#include <stdint.h>

#define userIdType uint64_t
#define sessionIdType uint64_t
#define sessionListVersionType uint64_t

class Account
{
private:
public:
        Account();
        userIdType id;
        sessionListVersionType sessionListVersion;
        std::string googleId;
        std::string username;
        std::string passHash;
        std::string email;

        ~Account();
};