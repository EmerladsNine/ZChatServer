#include "utils.h"
#include <cstdint>

uint16_t readUint16FromBuffer(std::vector<char> &buffer,int start) {
  return buffer[start] << 8 | buffer[start + 1];
}

uint32_t readUint32FromBuffer(std::vector<char> &buffer,int start)
{
        return buffer[start] << 24 | buffer[start + 1] << 16 | buffer[start + 2] << 8 | buffer[start + 3];
}
