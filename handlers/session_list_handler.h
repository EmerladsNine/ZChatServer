#include "../client.h"
#include "../protocol.h"

class SessionListHandler
{
private:
public:
        static void handleSessionListRequest(Client &client, expected_size expectedSize, Services &services);
};