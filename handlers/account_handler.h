#pragma once
#include "../client.h"

class AccountHandler
{
private:
public:
        static void EmailSignIn(Client &client, size_t expectedSize, Services &services);
        static void EmailSignUp(Client &client, size_t expectedSize, Services &services);
        static void GoogleSignIn(Client &client, size_t expectedSize, Services &services);
        static void GoogleSignUp(Client &client, size_t expectedSize, Services &services);
        static void SearchWithUsername(Client &client, size_t expectedSize, Services &services);
        static void SearchWithId(Client &client, size_t expectedSize, Services &services);
};