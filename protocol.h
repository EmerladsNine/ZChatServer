#pragma once
#include "client.h"
#include <iostream>

const uint16_t TIME_STAMP_BYTES = 8;
const uint16_t EXPECTED_SIZE_BYTES = 2;
const uint16_t HEAD_SIZE = 1;

const size_t HEADER_OFFSET = EXPECTED_SIZE_BYTES;
const size_t TIME_STAMP_OFFSET = HEADER_OFFSET + HEAD_SIZE;

const size_t USERNAME_LENGTH_MAX = 20;
const size_t PASSWORD_LENGTH_MAX = 254;
const size_t EMAIL_LENGTH_MAX = 254;
const size_t USERNAME_LENGTH_MIN = 1;
const size_t PASSWORD_LENGTH_MIN = 8;
const size_t EMAIL_LENGTH_MIN = 5;

class Protocol
{
private:
public:
        Protocol();
        static bool parseUnit(Client &client, Services &services);
        static void handleUnit(Client &client, size_t expectedSize, Services &services);
        static void handleNormalMessage(Client &client, size_t expectedSize, Services &services);
        ~Protocol();
};
