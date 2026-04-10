#pragma once
#include "client.h"
#include <iostream>

typedef uint32_t expected_size;

const uint16_t EXPECTED_SIZE_BYTES = sizeof(expected_size);
const uint16_t HEAD_SIZE = 1;

const size_t HEADER_OFFSET = EXPECTED_SIZE_BYTES;

const size_t USERNAME_LENGTH_MAX = 20;
const size_t PASSWORD_LENGTH_MAX = 128;
const size_t EMAIL_LENGTH_MAX = 254;
const size_t USERNAME_LENGTH_MIN = 1;
const size_t PASSWORD_LENGTH_MIN = 12;
const size_t EMAIL_LENGTH_MIN = 5;

class Protocol
{
private:
public:
        Protocol();
        static bool parseUnit(Client &client, Services &services);
        static void handleUnit(Client &client, expected_size expectedSize, Services &services);
        static void handleNormalMessage(Client &client, expected_size expectedSize, Services &services);
        ~Protocol();
};
