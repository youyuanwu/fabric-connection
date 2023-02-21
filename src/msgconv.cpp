#include "msgconv.hpp"
// #include "servicefabric/transport_message.hpp"
#include "chunk.hpp"

#include "fabrictransport_.h"
#include <boost/assert.hpp>
#include <moderncom/interfaces.h>
// namespace sf = servicefabric;

// bytes encoding:
// int32 len
// char[] data
// repeat
//
// first is the header
// remaining are repeated body

// transport msg needs to find correct ptr location to return

namespace fabricconnection {

class transport_message
    : public belt::com::object<transport_message, IFabricTransportMessage> {
public:
  // data chunks
  transport_message() : init_(false), data_(), buffers_(), buffer_ptrs_() {}

  // after init the msg should be valid
  bool init(std::string data) {
    data_ = std::move(data);
    if (data_.size() < 4) {
      return false;
    }

    std::size_t index = 0;
    // decode header
    {

      std::uint32_t len = {};
      bool ok = decodeUInt32(data_.substr(0, 4), len);
      if (!ok) {
        return false;
      }
      index += 4;
      // TODO: guard big len
      header_.BufferSize = len;
      header_.Buffer = reinterpret_cast<BYTE *>(&data_[index]);
      static_assert(sizeof(BYTE) == sizeof(char));
      header_ptr_ = &header_;
      index += len;
      if (index > data_.size()) {
        return false;
      }
    }

    // decode body
    while (true) {
      if (index + 4 > data_.size()) {
        return false;
      }
      std::uint32_t len = {};
      bool ok = decodeUInt32(data_.substr(index, 4), len);
      if (!ok) {
        return false;
      }
      index += 4;
      buffers_.push_back(FABRIC_TRANSPORT_MESSAGE_BUFFER{
          len, reinterpret_cast<BYTE *>(&data_[index])});
      buffer_ptrs_.push_back(&buffers_.back());
      index += len;
      if (index > data_.size()) {
        return false;
      }
      if (index == data_.size()) {
        break;
      }
    }
    init_ = true;
    return true;
  }

  void STDMETHODCALLTYPE GetHeaderAndBodyBuffer(
      /* [out] */ const FABRIC_TRANSPORT_MESSAGE_BUFFER **headerBuffer,
      /* [out] */ ULONG *msgBufferCount,
      /* [out] */ const FABRIC_TRANSPORT_MESSAGE_BUFFER **MsgBuffers) override {

    assert(init_);
    assert(MsgBuffers != nullptr);
    assert(headerBuffer != nullptr);
    assert(msgBufferCount != nullptr);

    *headerBuffer = header_ptr_;
    *msgBufferCount = static_cast<ULONG>(buffers_.size());
    *MsgBuffers = buffer_ptrs_.front();
  }

  void STDMETHODCALLTYPE Dispose(void) override {}

private:
  bool init_;
  std::string data_;
  FABRIC_TRANSPORT_MESSAGE_BUFFER header_;
  FABRIC_TRANSPORT_MESSAGE_BUFFER *header_ptr_;
  std::vector<FABRIC_TRANSPORT_MESSAGE_BUFFER> buffers_;
  //
  std::vector<FABRIC_TRANSPORT_MESSAGE_BUFFER *> buffer_ptrs_;
};

// returns message constructed from data.
bool data_to_transport_msg(std::string data,
                           IFabricTransportMessage **message) {
  assert(message != nullptr);
  auto msg_temp = transport_message::create_instance();
  bool ok = msg_temp.obj()->init(std::move(data));
  if (!ok) {
    return false;
  }
  belt::com::com_ptr<IFabricTransportMessage> str =
      std::move(msg_temp).to_ptr();
  *message = str.detach();
  return true;
}

bool transport_msg_to_data(IFabricTransportMessage *message,
                           std::string &data) {
  assert(message != nullptr);
  std::string out;

  const FABRIC_TRANSPORT_MESSAGE_BUFFER *header = {};
  ULONG msgCount = {};
  const FABRIC_TRANSPORT_MESSAGE_BUFFER *body = {};

  message->GetHeaderAndBodyBuffer(&header, &msgCount, &body);
  assert(header != nullptr);
  if (msgCount != 0) {
    assert(body != nullptr);
  }
  // encode header
  std::string header_len = encodeUInt32(header->BufferSize);
  out += header_len;
  out +=
      std::string(reinterpret_cast<char *>(header->Buffer), header->BufferSize);

  // encode body
  for (ULONG i = 0; i < msgCount; i++) {
    const FABRIC_TRANSPORT_MESSAGE_BUFFER *buff = body + i;
    std::string buff_len = encodeUInt32(buff->BufferSize);
    out += buff_len;
    out +=
        std::string(reinterpret_cast<char *>(buff->Buffer), buff->BufferSize);
  }

  data = std::move(out);
  return true;
}

} // namespace fabricconnection