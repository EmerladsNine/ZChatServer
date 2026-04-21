#pragma once
#include "../client.h"
#include "../protocol.h"

class FcmHandler
{
private:
public:
        static void handleFcmToken(Client &client, expected_size expectedSize, Services &services);
};
