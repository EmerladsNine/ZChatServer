#include "sha256_hash.h"
#include <sodium.h>
#include <iostream>


SHA256Hash::SHA256Hash()
{
}

SHA256Hash::~SHA256Hash()
{
}

int SHA256Hash::Hash(const std::string &plainText, std::string &HashedOut)
{
        if (!valid)
                return 0;
        HashedOut.resize(crypto_hash_sha256_BYTES);
        if (crypto_hash_sha256(
                reinterpret_cast<unsigned char*>(HashedOut.data()), reinterpret_cast<const unsigned char*>(plainText.data()),
                 plainText.size()) != 0)
        {
                std::cerr << "Out of memory" << std::endl;
                return 0;
        }
        return 1;
}

int SHA256Hash::Verify(const std::string &plainText, const std::string &hash)
{
    if (!valid)
        return 0;

    std::string computedHash;
    if (this->Hash(plainText, computedHash) == 0)
        return 0;

    if (computedHash.size() != crypto_hash_sha256_BYTES || 
        hash.size() != crypto_hash_sha256_BYTES)
        return 0;

    if (crypto_verify_32(
            reinterpret_cast<const unsigned char*>(computedHash.data()),
            reinterpret_cast<const unsigned char*>(hash.data())) != 0)
        return 0;
    return 1;
}