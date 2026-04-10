#include "../client.h"
#include "../protocol.h"

class TokenHandler
{
private:
public:
        static void useAccessToken(Client &client, expected_size expectedSize, Services &services);
        static void useRefreshToken(Client &client, expected_size expectedSize, Services &services);
};