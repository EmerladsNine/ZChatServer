#include "utils.h"
#include <cstdint>

uint16_t readUint16FromBuffer(std::vector<char> &buffer) {
  return buffer[1] << 8 | buffer[2];
}
