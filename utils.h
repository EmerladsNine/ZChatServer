#include <cstdint>
#include <vector>

uint16_t readUint16FromBuffer(std::vector<char> &buffer);

template <typename T>
std::vector<char> intToBigEndian(T num) {

    std::vector<char> list;
    for (int i = sizeof(T) - 1; i != -1; i--) {
        list.push_back(static_cast<char>((num >> (i * 8)) & 0xff));
    }

    return list;
}
