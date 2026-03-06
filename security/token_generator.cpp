#include "token_generator.h"
#include <iostream>

TokenGenerator::TokenGenerator()
{
        std::cout << "Entropy for randomness used is : " << rd.entropy() << std::endl;
}

TokenGenerator::~TokenGenerator()
{
}

std::vector<std::uint8_t> TokenGenerator::secureRandomBytes(size_t n)
{
    std::vector<uint8_t> bytes;
    bytes.reserve(n);
    while (bytes.size() < n)
    {
        auto r = rd();

        for (size_t i = 0; i < sizeof(r) && bytes.size() < n; i++)
        {
            bytes.push_back((r >> (i * 8)) & 0xFF);
        }
    }
    return bytes;
}

std::vector<std::uint8_t> TokenGenerator::generateAccessToken()
{
        return secureRandomBytes(32);
}

std::vector<std::uint8_t> TokenGenerator::generateRefreshToken()
{
        return secureRandomBytes(64);
}