#pragma once
#include <string>

class SHA256Hash
{
private:
public:
        int valid = false;
        SHA256Hash();
        int Hash(const std::string &password, std::string &HashedPasswordOut);
        ~SHA256Hash();
};