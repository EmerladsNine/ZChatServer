#pragma once
#include "../client.h"
#include "../protocol.h"

class AccountHandler
{
private:
public:
        static bool isEmailValid(const std::string &email);
        static bool isUsernameValid(const std::string &username);
        static void EmailSignIn(Client &client, expected_size xpectedSize, Services &services);
        static void EmailSignUp(Client &client, expected_size expectedSize, Services &services);
        static void GoogleSignIn(Client &client, expected_size expectedSize, Services &services);
        static void GoogleSignUp(Client &client, expected_size expectedSize, Services &services);
        static void SearchWithUsername(Client &client, expected_size expectedSize, Services &services);
        static void SearchWithId(Client &client, expected_size expectedSize, Services &services);
};