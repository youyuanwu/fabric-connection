#include "chunk.hpp"

namespace fabricconnection {

// very naive impl of converting struct to a string

// format:
// 1 byte for type
// 4 bytes for id uint32
// 4 bytes for strlen
// content of strlen
//
// total len is not included by serialize, but should be written to wire
// separately

// assume msgout has be resized correctly
std::string encodeUInt32(std::uint32_t num) {
  std::string out;
  out.resize(4);
  // encode 4 byte length in big endian starting at begin char
  int j = 3;
  for (int i = 0; i <= 3; i++) {
    out[j] = (num >> (8 * i)) & 0xff;
    j--;
  }
  return out;
}

std::string encodeType(chunk::type tp) {
  std::string out;
  out.resize(1);
  char chtp = static_cast<char>(tp);
  out[0] = chtp;
  return out;
}

bool decodeType(const char ctype, chunk::type &ret) {
  if (ctype == chunk::type::req) {
    ret = chunk::type::req;
    return true;
  } else if (ctype == chunk::type::oneway) {
    ret = chunk::type::oneway;
    return true;
  } else {
    return false;
  }
}

bool decodeUInt32(std::string data, std::uint32_t &ret) {
  if (data.size() != 4) {
    return false;
  }
  std::uint32_t len = 0;
  len |= (std::uint8_t)data[3];
  len |= (std::uint8_t)data[2] << 8;
  len |= (std::uint8_t)data[1] << 16;
  len |= (std::uint8_t)data[0] << 24;
  ret = len;
  return true;
}

std::string chunk::Serialize() const {
  std::string ret;
  std::uint32_t data_size = static_cast<std::uint32_t>(data.size());
  std::uint32_t total_len = 1 + 4 + 4 + data_size;
  ret.reserve(total_len);
  // ret += encodeUInt32(total_len);
  ret += encodeType(chunk_type);
  ret += encodeUInt32(id);
  ret += encodeUInt32(data_size);
  ret += data;
  return ret;
}

bool chunk::Deserialize(std::string const &content) {
  bool ok = true;
  if (content.size() < 4 + 1 + 4 + 4) {
    return false;
  }
  int index = 0;
  // std::uint32_t total_len = 0;
  // ok = decodeUInt32(content.substr(index, 4), total_len);
  //  if (!ok) {
  //    return false;
  //  }
  //  if (total_len != content.length()) {
  //    return false;
  //  }
  //  index += 4;
  ok = decodeType(content[index], chunk_type);
  if (!ok) {
    return false;
  }
  index += 1;
  ok = decodeUInt32(content.substr(index, 4), id);
  if (!ok) {
    return false;
  }
  index += 4;
  std::uint32_t len = 0;
  ok = decodeUInt32(content.substr(index, 4), len);
  index += 4;
  if (len + 1 + 4 + 4 != content.size()) {
    return false;
  }
  data = content.substr(index);
  return true;
}

} // namespace fabricconnection