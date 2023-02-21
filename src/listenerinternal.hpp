#pragma once
#include "namedpipe_server.hpp"

#include <thread>

namespace fabricconnection {

namespace net = boost::asio;
namespace winnet = boost::winasio;

class listener_internal {
public:
  // server runs when obj is constructed
  listener_internal(std::string addr,
                    IFabricTransportMessageHandler *requestHandler,
                    IFabricTransportConnectionHandler *connectionHandler)
      : ioc_(), svr_(ioc_, addr, requestHandler, connectionHandler),
        is_running_(false) {}

  void run_async() {
    th_ = std::jthread::jthread([this]() { ioc_.run(); });
    is_running_ = true;
  }

  void stop() {
    assert(is_running_);
    ioc_.stop();
    is_running_ = false;
    th_.join();
  }

private:
  net::io_context ioc_;
  server_movable svr_;
  std::jthread th_;
  bool is_running_;
};

} // namespace fabricconnection