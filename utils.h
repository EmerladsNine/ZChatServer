#include <cstdint>
#include <vector>

uint16_t readUint16FromBuffer(std::vector<char> &buffer);

template <typename T>
std::vector<char> intToBigEndian(std::T num);
