#pragma once
#include "../client.h"

class GoogleAuth
{
private:
public:
        GoogleAuth();
        static void StartAuthentication(Client &client, size_t expectedSize);
        ~GoogleAuth();
};
