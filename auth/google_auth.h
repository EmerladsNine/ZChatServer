#pragma once
#include "../client.h"
#include "../protocol.h"

const size_t GOOGLE_TOKEN_SIGNIN_OFFSET = HEADER_OFFSET + HEAD_SIZE;

class GoogleAuth
{
private:
public:
        GoogleAuth();
        static void SignIn(Client &client, size_t expectedSize);
        static void SignUp(Client &client, size_t expectedSize);
        ~GoogleAuth();
};
