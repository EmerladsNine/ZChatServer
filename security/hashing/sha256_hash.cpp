#include "sha256_hash.h"
#include <sodium.h>
#include <iostream>


SHA256Hash::SHA256Hash()
{
}

SHA256Hash::~SHA256Hash()
{
}

int SHA256Hash::Hash(const std::string &text, std::string &HashedtextOut)
{
        if (!valid)
                return 0;
        HashedtextOut.resize(crypto_hash_sha256_BYTES);
        if (crypto_hash_sha256(
                reinterpret_cast<unsigned char*>(HashedtextOut.data()), reinterpret_cast<const unsigned char*>(text.data()),
                 text.size()) != 0)
        {
                std::cerr << "Out of memory" << std::endl;
                return 0;
        }
        return 1;
}