#pragma once
#include "client.h"

const uint16_t TIME_STAMP_BYTES = 8;
const uint16_t EXPECTED_SIZE_BYTES = 2;
const uint16_t HEAD_SIZE = 1;

const size_t HEADER_OFFSET = EXPECTED_SIZE_BYTES;
const size_t TIME_STAMP_OFFSET = HEADER_OFFSET + HEAD_SIZE;

class Protocol
{
private:
public:
        Protocol();
        static bool parseUnit(Client &client);
        static void handleNormalMessage(Client &client);
        ~Protocol();
};
