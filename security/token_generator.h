#include <vector>
#include <random>

class TokenGenerator
{
private:
public:
        TokenGenerator();
        ~TokenGenerator();
        std::random_device rd;
        std::vector<std::uint8_t> secureRandomBytes(size_t n);
        std::vector<std::uint8_t> generateAccessToken();
        std::vector<std::uint8_t> generateRefreshToken();
};
