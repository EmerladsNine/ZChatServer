#include "../client.h"

class TokenHandler
{
private:
public:
        TokenHandler();
        static void useAccessToken(Client &client, size_t expectedSize, Services &services);
        static void useRefreshToken(Client &client, size_t expectedSize, Services &services);
        ~TokenHandler();
};