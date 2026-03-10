#pragma once
#include <string>

class Account
{
private:
public:
        Account();
        int id;
        std::string googleId;
        std::string username;
        std::string passHash;
        std::string email;
        ~Account();
};