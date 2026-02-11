#include "argon_hash.h"
#include <sodium.h>
#include <iostream>
#include <string>

ArgonHash::ArgonHash()
{
        if (sodium_init() < 0)
        {
                std::cerr << "Failed To Initialize Sodium" << std::endl;
                return;
        }
        valid = true;
}

ArgonHash::~ArgonHash()
{
}

int ArgonHash::Hash(const std::string &password, std::string &HashedPasswordOut)
{
        if (!valid)
                return 0;
        HashedPasswordOut.resize(crypto_pwhash_STRBYTES);
        if (crypto_pwhash_str(
                HashedPasswordOut.data(), password.c_str(), password.length(),
                crypto_pwhash_OPSLIMIT_MODERATE,
                crypto_pwhash_MEMLIMIT_MODERATE) != 0)
        {
                std::cerr << "Out of memory" << std::endl;
                return 0;
        }
        return 1;
}

int ArgonHash::verifyPassword(const char *password, size_t passwordLength, const char *hash, int *isEqual_OUT)
{
        if (!valid)
                return 0;
        if (crypto_pwhash_str_verify(hash, password, passwordLength) != 0)
        {
                *isEqual_OUT = 0;
        }
        else
        {
                *isEqual_OUT = 1;
        }
        return 1;
}