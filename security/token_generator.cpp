#include "token_generator.h"
#include <iostream>

TokenGenerator::TokenGenerator()
{
        std::cout << "Entropy for randomness used is : " << rd.entropy() << std::endl;
}

TokenGenerator::~TokenGenerator()
{
}

std::vector<char> TokenGenerator::secureRandomBytes(size_t n)
{
    std::vector<char> bytes;
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

std::vector<char> TokenGenerator::generateAccessToken()
{
        return secureRandomBytes(32);
}

std::vector<char> TokenGenerator::generateRefreshToken()
{
        return secureRandomBytes(64);
}