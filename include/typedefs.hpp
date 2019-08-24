#pragma once

#include <array>
#include <functional>
#include <vector>

template <int T>
using Chars = std::array<char, T>;

template <int T>
using UChars = std::array<unsigned char, T>;

typedef std::vector<uint8_t > UCharsVec;
