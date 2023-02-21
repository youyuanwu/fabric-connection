#pragma once

#include <string>

namespace fabricconnection {

// can be request or oneway
// note that client and server can send and recieve request data or oneway
class chunk {
public:
  enum type { req, oneway };
  type chunk_type;
  std::uint32_t id;
  std::string data;

  // convert to wire format
  std::string Serialize() const;
  // convert from wire format
  // return true if success
  bool Deserialize(std::string const &data);
};

// conversion functions:
bool decodeUInt32(std::string data, std::uint32_t &ret);

std::string encodeUInt32(std::uint32_t num);

} // namespace fabricconnection