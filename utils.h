#pragma once
#include <cstdint>
#include <vector>

template <typename T>
std::vector<char> intToBigEndian(T num)
{

        std::vector<char> list;
        for (int i = sizeof(T) - 1; i != -1; i--)
        {
                list.push_back(static_cast<char>((num >> (i * 8)) & 0xff));
        }

        return list;
}

template <typename T>
T bigEndianToInt(std::vector<char> &buffer, int start)
{
        T i;
        int j = 0;
        for (int i = sizeof(T) - 1; i != -1; i--)
        {
                i |= buffer[start + j] << i * 8;
                j++;
        }
        return i;
}