#include <vector>
#include <random>

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
};
