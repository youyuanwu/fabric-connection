#pragma once
// this header is intended to be directly included by listener.cpp for now.

#include "namedpipe_server.hpp"

#include "boost/asio.hpp"
#include "boost/winasio/named_pipe/named_pipe_protocol.hpp"
#include "fabrictransport_.h"

#include "servicefabric/waitable_callback.hpp"

#include "chunk.hpp"
#include "msgconv.hpp"

namespace fabricconnection {

namespace net = boost::asio;
namespace winnet = boost::winasio;

namespace sf = servicefabric;

// protocol:
// after accept the stream of a client, start asycn read and write chunks in a
// session chunks carries ids, where client sent with id 1, session pass the
// content to the user handler. User handler returns async msg, and session
// write back to client with id 1. Server can also send oneway chunk.

class session2 : public std::enable_shared_from_this<session2> {
public:
  session2(
      winnet::named_pipe_protocol<net::io_context::executor_type>::pipe socket,
      IFabricTransportMessageHandler *requestHandler)
      : socket_(std::move(socket)) {
    std::cout << "session constructed" << std::endl;
    requestHandler->AddRef();
    requestHandler_.Attach(requestHandler);
  }

  void start() {
    // TODO: invoke connection handler.this may require packet change

    read_chunck_len();
  }

private:
  void read_chunck_len() {
    auto self(shared_from_this());
    net::async_read(socket_, net::buffer(chunck_len_, sizeof(chunck_len_)),
                    [this, self](std::error_code ec, std::size_t length) {
                      if (ec) {
                        // error abort the connection
                        // TODO:
                        return;
                      }
                      assert(length == sizeof(chunck_len_));
                      DBG_UNREFERENCED_LOCAL_VARIABLE(length);

                      // validate chunck len
                      std::uint32_t chunck_len = {};
                      bool ok =
                          decodeUInt32(std::string(chunck_len_, 4), chunck_len);
                      if (!ok) {
                        return;
                      }
                      read_chunck(chunck_len);
                    });
  }

  void read_chunck(std::uint32_t len) {
    // check len. If client passes too big of len reject.
    const std::int32_t max_len = 1048576; // 1 mb TODO: let user configure this.
    if (len > max_len) {
      return;
    }
    chunck_buff_.resize(len);
    auto self(shared_from_this());
    net::async_read(
        socket_, net::buffer(chunck_buff_, len),
        [this, self, len](std::error_code ec, std::size_t length) {
          if (ec) {
            return;
          }
          assert(length == len);
          DBG_UNREFERENCED_LOCAL_VARIABLE(length);

          // got chunk_buff need to parse it
          chunk ck = {};
          bool ok = ck.Deserialize(chunck_buff_);
          if (!ok) {
            return;
          }
          CComPtr<IFabricTransportMessage> msg;
          // convert chunk data into fabric msg
          ok = data_to_transport_msg(std::move(ck.data), &msg);
          if (!ok) {
            return;
          }
          // TODO: process the request.
          if (ck.chunk_type == chunk::type::oneway) {
            // TODO: generate id
            requestHandler_->HandleOneWay(L"id", msg);
            // read the next chunk
            read_chunck_len();
          } else if (ck.chunk_type == chunk::type::req) {
            belt::com::com_ptr<sf::IFabricAsyncOperationWaitableCallback>
                callback =
                    sf::FabricAsyncOperationWaitableCallback::create_instance()
                        .to_ptr();
            belt::com::com_ptr<IFabricAsyncOperationContext> ctx;
            HRESULT hr = requestHandler_->BeginProcessRequest(
                L"id", msg, 1000, callback.get(), ctx.put());
            if (hr != S_OK) {
              return;
            }
            callback->Wait();
            belt::com::com_ptr<IFabricTransportMessage> reply;
            hr = requestHandler_->EndProcessRequest(ctx.get(), reply.put());
            if (hr != S_OK) {
              return;
            }
            write_chunk_len(reply, ck.id);
            // async write stuff.
          } else {
            assert(false);
          }
        });
  }

  void write_chunk_len(belt::com::com_ptr<IFabricTransportMessage> reply,
                       std::uint32_t id) {
    // reuse chunk_len and chunk_buff
    std::string data;
    bool ok = transport_msg_to_data(reply.get(), data);
    if (!ok) {
      return;
    }
    chunk ck = {};
    ck.chunk_type = chunk::type::req;
    ck.id = id;
    ck.data = std::move(data);

    chunck_buff_ = ck.Serialize();
    std::string lenstr =
        encodeUInt32(static_cast<std::uint32_t>(chunck_buff_.size()));
    auto err = memcpy_s(chunck_len_, 4, lenstr.c_str(), 4);
    assert(err == 0);
    DBG_UNREFERENCED_LOCAL_VARIABLE(err);
    auto self(shared_from_this());
    net::async_write(socket_, net::buffer(chunck_len_, 4),
                     [this, self](std::error_code ec, std::size_t length) {
                       if (ec) {
                         return;
                       }
                       assert(length == 4);
                       DBG_UNREFERENCED_LOCAL_VARIABLE(length);
                       write_chunk();
                     });
  }

  void write_chunk() {
    auto self(shared_from_this());
    net::async_write(socket_, net::buffer(chunck_buff_, chunck_buff_.size()),
                     [this, self](std::error_code ec, std::size_t length) {
                       if (ec) {
                         return;
                       }
                       assert(length == chunck_buff_.size());
                       DBG_UNREFERENCED_LOCAL_VARIABLE(length);
                       // read next request
                       read_chunck_len();
                     });
  }

  winnet::named_pipe_protocol<net::io_context::executor_type>::pipe socket_;
  char chunck_len_[4];
  std::string chunck_buff_;
  CComPtr<IFabricTransportMessageHandler> requestHandler_;
};

server_movable::server_movable(
    net::io_context &io_context,
    winnet::named_pipe_protocol<net::io_context::executor_type>::endpoint ep,
    IFabricTransportMessageHandler *requestHandler,
    IFabricTransportConnectionHandler *connectionHandler)
    : acceptor_(io_context, ep) {
  requestHandler->AddRef();
  requestHandler_.Attach(requestHandler);
  connectionHandler->AddRef();
  connectionHandler_.Attach(connectionHandler);

  do_accept();
}

void server_movable::do_accept() {
  std::cout << "do_accept" << std::endl;
  acceptor_.async_accept([this](
                             boost::system::error_code ec,
                             winnet::named_pipe_protocol<
                                 net::io_context::executor_type>::pipe socket) {
    if (!ec) {
      std::cout << "do_accept handler ok. making session" << std::endl;
      std::make_shared<session2>(std::move(socket), requestHandler_)->start();
    } else {
      // TODO: bail out to avoid infinite loop
      std::cout << "accept handler error: " << ec.message() << std::endl;
      return;
    }

    do_accept();
  });
}

} // namespace fabricconnection