#pragma once

#include <array>
#include <fstream>
#include <functional>

template <int T>
using Chars = std::array<char, T>;

template <int T>
using UChars = std::array<unsigned char, T>;

typedef std::vector<unsigned char>  UCharsVec;

/**
 * Checks whether or not the remaining stream
 * includes an optional color table or not
 * @param stream
 * @return bool
 */
bool hasColorTable(std::ifstream &stream);

/**
 * Consumes a single byte of [gif::EXTENSION_INTRODUCER] from the stream
 * @throws std::runtime_error
 */
void consumeExtIntroducer(std::ifstream &);

void withSubBlocks(std::ifstream &,
                   const std::function<void(UCharsVec &, unsigned char)> &);

/**
 * Consumes a block label that's a compile time constant
 * @throws std::runtime_error
 * @tparam LABEL
 */
template <int LABEL>
void consumeLabel(std::ifstream &stream) {
  const unsigned char extractedLabel = stream.get();
  if(extractedLabel != LABEL) {
    throw std::runtime_error(
        "Consumed label " + std::to_string(extractedLabel) +
        " doesn't match expected constant " + std::to_string(LABEL));
  }
}
