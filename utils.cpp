#include "utils.h"
#include <cstdint>

uint16_t readUint16FromBuffer(std::vector<char> &buffer,int start) {
  return buffer[start] << 8 | buffer[start + 1];
}
