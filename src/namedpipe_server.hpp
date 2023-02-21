#pragma once
// this header is intended to be directly included by listener.cpp for now.

#include "boost/asio.hpp"
#include "boost/winasio/named_pipe/named_pipe_protocol.hpp"

#include "fabrictransport_.h"
#include <atlbase.h>
#include <atlcom.h>

namespace fabricconnection {

namespace net = boost::asio;
namespace winnet = boost::winasio;

class server_movable {
public:
  server_movable(
      net::io_context &io_context,
      winnet::named_pipe_protocol<net::io_context::executor_type>::endpoint ep,
      IFabricTransportMessageHandler *requestHandler,
      IFabricTransportConnectionHandler *connectionHandler);

private:
  void do_accept();

  winnet::named_pipe_protocol<net::io_context::executor_type>::acceptor
      acceptor_;
  CComPtr<IFabricTransportMessageHandler> requestHandler_;
  CComPtr<IFabricTransportConnectionHandler> connectionHandler_;
};

} // namespace fabricconnection