#pragma once
#include "../client.h"
#include "../protocol.h"

const uint16_t EMAIN_LENGTH_SIZE = 1;
const uint16_t PASSWORD_LENGTH_SIZE = 1;
const size_t EMAIL_LENGTH_OFFSET = HEADER_OFFSET + HEAD_SIZE;
const size_t EMAIL_OFFSET = EMAIL_LENGTH_OFFSET + EMAIN_LENGTH_SIZE;

class EmailAuth
{
private:
public:
        EmailAuth();
        static void SignIn(Client &client, size_t expectedSize, Services &services);
        static void SignUp(Client &client, size_t expectedSize, Services &services);
        ~EmailAuth();
};
