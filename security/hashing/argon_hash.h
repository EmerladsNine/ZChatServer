#pragma once
#include <string>

class ArgonHash
{
private:
public:
        int valid = false;
        ArgonHash();
        int Hash(const std::string &password, std::string &HashedPasswordOut);
        int verifyPassword(const char *password, size_t passwordLength, const char *hash, bool& isEqual_OUT);
        ~ArgonHash();
};
