#pragma once
#include "argon_hash.h"
#include "sha256_hash.h"

class HashManager
{
private:
public:
        HashManager();
        ArgonHash argonHash;
        SHA256Hash sha256Hash;
        ~HashManager();
};