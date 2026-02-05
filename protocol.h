#pragma once
#include "client.h"
#include <iostream>

const uint16_t TIME_STAMP_BYTES = 8;
const uint16_t EXPECTED_SIZE_BYTES = 2;
const uint16_t HEAD_SIZE = 1;
const uint16_t EMAIN_LENGTH_SIZE = 1;
const uint16_t PASSWORD_LENGTH_SIZE = 1;

const size_t HEADER_OFFSET = EXPECTED_SIZE_BYTES;
const size_t TIME_STAMP_OFFSET = HEADER_OFFSET + HEAD_SIZE;
const size_t EMAIL_LENGTH_OFFSET = HEADER_OFFSET + HEAD_SIZE;
const size_t EMAIL_OFFSET = EMAIL_LENGTH_OFFSET + EMAIN_LENGTH_SIZE;

class Protocol
{
private:
public:
        Protocol();
        static bool parseUnit(Client &client);
        static void handleUnit(Client &client, size_t expectedSize);
        static void handleNormalMessage(Client &client, size_t expectedSize);
        ~Protocol();
};
