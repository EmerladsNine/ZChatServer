#pragma once
#include "../client.h"
#include "../protocol.h"

class EmailAuth
{
private:
public:
        EmailAuth();
        static void SignIn(Client &client, size_t expectedSize);
        static void SignUp(Client &client, size_t expectedSize);
        ~EmailAuth();
};
