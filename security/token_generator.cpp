#include "token_generator.h"
#include "../services.h"
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

bool TokenGenerator::generateSession(Services &services, std::vector<char> &accessToken, std::vector<char> &refreshToken, std::string &accessTokenHash, std::string &refreshTokenHash)
{
        std::string accessTokenText;
        std::string refreshTokenText;
        for (int i = 0; i < 10; i++)
        {
                accessToken = generateAccessToken();
                refreshToken = generateRefreshToken();
                accessTokenText.assign(accessToken.begin(), accessToken.end());
                refreshTokenText.assign(refreshToken.begin(), refreshToken.end());
                services.hashManager.argonHash.Hash(refreshTokenText, refreshTokenHash);
                services.hashManager.sha256Hash.Hash(accessTokenText, accessTokenHash);
                bool exist;
                if (!services.db.sessionExists(accessTokenHash, refreshTokenHash.data(), exist))
                        return false;
                if (!exist)
                        return true;
        }
        return false;
}