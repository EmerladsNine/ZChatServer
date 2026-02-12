#pragma once
#include "../client.h"
#include "../protocol.h"

// Sign in
const size_t GOOGLE_TOKEN_SIGNIN_OFFSET = HEADER_OFFSET + HEAD_SIZE;
// Sign up
const size_t USERNAME_LENGTH_SIZE = 1;
const size_t USERNAME_LENGTH_OFFSET = HEADER_OFFSET + HEAD_SIZE;
const size_t USERNAME_OFFSET = USERNAME_LENGTH_OFFSET + USERNAME_LENGTH_SIZE;

class GoogleAuth
{
private:
public:
        GoogleAuth();
        static void SignIn(Client &client, size_t expectedSize, Services &services);
        static void SignUp(Client &client, size_t expectedSize, Services &services);
        ~GoogleAuth();
};
