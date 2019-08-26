#include "decompressor.hpp"

// std::vector<gif::ImageBlock> extractFrames(const std::vector<gif::ImageBody>&
// data) {
//  std::vector<gif::ImageBlock> frameBits;
//  std::copy_if(data.begin(), data.end(), std::back_inserter(frameBits),
//  [](const auto& var) {
//    return std::holds_alternative<gif::ImageBlock>(var);
//  });
//  return frameBits;
//}
//

std::unordered_map<std::string, uint8_t> buildTable(const std::string& chars) {
  std::unordered_map<std::string, uint8_t> dict;
  int i = 0;
  for(const auto& c : chars) {
    std::string e({ c });
    const auto hasKey = dict.find(e) != dict.end();
    if(hasKey) {
      continue;
    }
    dict[e] = i++;
  }
  return dict;
}
std::vector<uint8_t> compress(const std::string& chars) {
  std::unordered_map<std::string, uint8_t> dict = buildTable(chars);
  std::string s;
  std::vector<uint8_t> output;
  auto i = dict.size();
  for(const auto& c : chars) {
    const auto hasKey = dict.find(s + c) != dict.end();
    if(hasKey) {
      s += c;
    } else {
      output.push_back(dict[s]);
      dict[s + c] = i++;
      s           = c;
    }
  }
  output.push_back(dict[s]);
  return output;
}

gif::DecompressedImageData mapImage(const gif::CompressedImageData& img) {
  auto e = img;
  return gif::DecompressedImageData();
}

void decompress(const gif::CompressedImageData& data) {
  const auto blocks = data.subBlocks;
  //  std::vector<gif::DecompressedImageData> vect;
  //  std::transform(data.compressedImageData.begin(),
  //                 data.compressedImageData.end(), vect.begin(), [](auto& img)
  //                 {
  //    if (std::holds_alternative<gif::CompressedImageData>(img)) {
  //      return
  //    }
  //  });
}
