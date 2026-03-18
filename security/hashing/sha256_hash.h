#pragma once
#include <string>

class SHA256Hash
{
private:
public:
        int valid = false;
        SHA256Hash();
        int Hash(const std::string &plainText, std::string &HashedOut);
        int Verify(const std::string &plainText, const std::string &hash);
        ~SHA256Hash();
};