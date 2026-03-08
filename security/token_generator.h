#pragma once
#include <vector>
#include <random>
#include <string>

class Services;
class TokenGenerator
{
private:
public:
        TokenGenerator();
        ~TokenGenerator();
        std::random_device rd;
        std::vector<char> secureRandomBytes(size_t n);
        std::vector<char> generateAccessToken();
        std::vector<char> generateRefreshToken();
        bool generateSession(Services &services, std::vector<char> &accessToken, std::vector<char> &refreshToken, std::string &accessTokenHash, std::string &refreshTokenHash);
};
