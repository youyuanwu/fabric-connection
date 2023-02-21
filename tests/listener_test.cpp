#define BOOST_TEST_MODULE listener
#include <boost/test/unit_test.hpp>

#include "fabricconnection/connection.hpp"
#include "servicefabric/async_context.hpp"
#include "servicefabric/transport_dummy_server_conn_handler.hpp"
#include "servicefabric/transport_message.hpp"
#include "servicefabric/waitable_callback.hpp"

#include "boost/asio.hpp"
#include "boost/winasio/named_pipe/named_pipe_protocol.hpp"

#include "chunk.hpp"
#include "msgconv.hpp"

namespace sf = servicefabric;
namespace winnet = boost::winasio;
namespace net = boost::asio;

class request_handler
    : public belt::com::object<request_handler,
                               IFabricTransportMessageHandler> {
public:
  HRESULT STDMETHODCALLTYPE BeginProcessRequest(
      /* [in] */ COMMUNICATION_CLIENT_ID clientId,
      /* [in] */ IFabricTransportMessage *message,
      /* [in] */ DWORD timeoutMilliseconds,
      /* [in] */ IFabricAsyncOperationCallback *callback,
      /* [retval][out] */ IFabricAsyncOperationContext **context) override {
    UNREFERENCED_PARAMETER(clientId);
    UNREFERENCED_PARAMETER(message);
    UNREFERENCED_PARAMETER(timeoutMilliseconds);
    belt::com::com_ptr<IFabricAsyncOperationContext> ctx =
        sf::async_context::create_instance(callback).to_ptr();

    *context = ctx.detach();
    return S_OK;
  }

  HRESULT STDMETHODCALLTYPE EndProcessRequest(
      /* [in] */ IFabricAsyncOperationContext *context,
      /* [retval][out] */ IFabricTransportMessage **reply) override {
    UNREFERENCED_PARAMETER(context);
    belt::com::com_ptr<IFabricTransportMessage> msg =
        sf::transport_message::create_instance("mybody", "myheader").to_ptr();
    *reply = msg.detach();
    return S_OK;
  }

  HRESULT STDMETHODCALLTYPE HandleOneWay(
      /* [in] */ COMMUNICATION_CLIENT_ID clientId,
      /* [in] */ IFabricTransportMessage *message) override {
    UNREFERENCED_PARAMETER(clientId);
    UNREFERENCED_PARAMETER(message);
    std::cout << "HandleOneWay: " << std::endl;
    return S_OK;
  }
};

BOOST_AUTO_TEST_SUITE(test_listener)

BOOST_AUTO_TEST_CASE(listener) {

  fabricconnection::FabricConnectionSettings settings = {};
  settings.Address = "\\\\.\\pipe\\mynamedpipe";

  belt::com::com_ptr<IFabricTransportConnectionHandler> conn_handler =
      sf::transport_dummy_server_conn_handler::create_instance().to_ptr();

  belt::com::com_ptr<IFabricTransportMessageHandler> request_handler =
      request_handler::create_instance().to_ptr();

  belt::com::com_ptr<IFabricTransportListener> listener;
  HRESULT hr = fabricconnection::CreateFabricConnectionListener(
      &settings, request_handler.get(), conn_handler.get(), listener.put());
  BOOST_REQUIRE_EQUAL(hr, S_OK);

  {
    belt::com::com_ptr<sf::IFabricAsyncOperationWaitableCallback> callback =
        sf::FabricAsyncOperationWaitableCallback::create_instance().to_ptr();
    belt::com::com_ptr<IFabricAsyncOperationContext> ctx;
    hr = listener->BeginOpen(callback.get(), ctx.put());
    BOOST_REQUIRE_EQUAL(hr, S_OK);
    callback->Wait();
    belt::com::com_ptr<IFabricStringResult> msg;
    hr = listener->EndOpen(ctx.get(), msg.put());
    BOOST_REQUIRE_EQUAL(hr, S_OK);
    BOOST_REQUIRE(msg->get_String() ==
                  std::wstring(L"\\\\.\\pipe\\mynamedpipe"));
  }

  // std::this_thread::sleep_for(std::chrono::seconds(2));

  // write to pipe manually.
  net::io_context io_context;
  winnet::named_pipe_protocol<net::io_context::executor_type>::endpoint ep(
      "\\\\.\\pipe\\mynamedpipe");
  winnet::named_pipe_protocol<net::io_context::executor_type>::pipe pipe(
      io_context);
  boost::system::error_code ec = {};
  pipe.connect(ep, ec);
  BOOST_REQUIRE(!ec.failed());

  // one way msg.
  {
    belt::com::com_ptr<IFabricTransportMessage> msg =
        sf::transport_message::create_instance("mybody", "myheader").to_ptr();
    std::string data;
    bool ok = fabricconnection::transport_msg_to_data(msg.get(), data);
    BOOST_REQUIRE(ok);
    fabricconnection::chunk ck = {};
    ck.chunk_type = fabricconnection::chunk::type::oneway;
    ck.id = 1;
    ck.data = std::move(data);
    std::string wire = ck.Serialize();
    std::string lenstr = fabricconnection::encodeUInt32(
        static_cast<std::uint32_t>(wire.length()));

    // write len
    net::write(pipe, net::buffer(lenstr, lenstr.size()), ec);
    BOOST_REQUIRE(!ec.failed());
    // write content
    net::write(pipe, net::buffer(wire, wire.size()), ec);
    BOOST_REQUIRE(!ec.failed());

    // This is one way call so no reply.
    // read len
    // std::string replylen;
    // replylen.resize(4);
    // net::read(pipe, net::buffer(replylen, replylen.size()), ec);
    // BOOST_REQUIRE(!ec.failed());
  }

  // normal request
  {
    belt::com::com_ptr<IFabricTransportMessage> msg =
        sf::transport_message::create_instance("mybody", "myheader").to_ptr();
    std::string data;
    bool ok = fabricconnection::transport_msg_to_data(msg.get(), data);
    BOOST_REQUIRE(ok);
    fabricconnection::chunk ck = {};
    ck.chunk_type = fabricconnection::chunk::type::req;
    ck.id = 1;
    ck.data = std::move(data);
    std::string wire = ck.Serialize();
    std::string lenstr = fabricconnection::encodeUInt32(
        static_cast<std::uint32_t>(wire.length()));

    // write len
    net::write(pipe, net::buffer(lenstr, lenstr.size()), ec);
    BOOST_REQUIRE(!ec.failed());
    // write content
    net::write(pipe, net::buffer(wire, wire.size()), ec);
    BOOST_REQUIRE(!ec.failed());

    // read len
    std::string replylenstr;
    replylenstr.resize(4);
    net::read(pipe, net::buffer(replylenstr, replylenstr.size()), ec);
    BOOST_REQUIRE(!ec.failed());
    std::uint32_t replylen = {};
    ok = fabricconnection::decodeUInt32(replylenstr, replylen);
    BOOST_REQUIRE(ok);
    std::string replywire;
    replywire.resize(replylen);
    net::read(pipe, net::buffer(replywire, replywire.size()), ec);
    BOOST_REQUIRE(!ec.failed());
    fabricconnection::chunk ck2;
    ok = ck2.Deserialize(replywire);
    BOOST_REQUIRE(ok);
    belt::com::com_ptr<IFabricTransportMessage> msg_reply;
    ok = fabricconnection::data_to_transport_msg(ck2.data, msg_reply.put());
    BOOST_REQUIRE(ok);

    const FABRIC_TRANSPORT_MESSAGE_BUFFER *header = {};
    ULONG msgCount = {};
    const FABRIC_TRANSPORT_MESSAGE_BUFFER *body = {};
    msg_reply->GetHeaderAndBodyBuffer(&header, &msgCount, &body);
    BOOST_CHECK(header != nullptr);
    BOOST_CHECK(msgCount == 1);
    BOOST_CHECK(body != nullptr);

    BOOST_CHECK_EQUAL("myheader",
                      std::string(reinterpret_cast<char *>(header->Buffer),
                                  header->BufferSize));
    BOOST_CHECK_EQUAL(
        "mybody",
        std::string(reinterpret_cast<char *>(body->Buffer), body->BufferSize));
  }

  {
    belt::com::com_ptr<sf::IFabricAsyncOperationWaitableCallback> callback =
        sf::FabricAsyncOperationWaitableCallback::create_instance().to_ptr();
    belt::com::com_ptr<IFabricAsyncOperationContext> ctx;
    hr = listener->BeginClose(callback.get(), ctx.put());
    BOOST_REQUIRE_EQUAL(hr, S_OK);
    callback->Wait();
    hr = listener->EndClose(ctx.get());
    BOOST_REQUIRE_EQUAL(hr, S_OK);
  }
}

BOOST_AUTO_TEST_CASE(chunk_test) {
  fabricconnection::chunk ck;
  ck.chunk_type = fabricconnection::chunk::oneway;
  ck.id = 111;
  ck.data = "mydata";

  std::string wire = ck.Serialize();

  fabricconnection::chunk ck2 = {};
  bool ok = ck2.Deserialize(wire);
  BOOST_REQUIRE(ok);
  BOOST_CHECK_EQUAL(ck.chunk_type, ck2.chunk_type);
  BOOST_CHECK_EQUAL(ck.id, ck2.id);
  BOOST_CHECK_EQUAL(ck.data, ck2.data);
}

BOOST_AUTO_TEST_CASE(conv_test) {
  belt::com::com_ptr<IFabricTransportMessage> msg =
      sf::transport_message::create_instance("mybody", "myheader").to_ptr();

  std::string data;
  {
    bool ok = fabricconnection::transport_msg_to_data(msg.get(), data);
    BOOST_REQUIRE(ok);
  }

  belt::com::com_ptr<IFabricTransportMessage> msg2;
  {
    bool ok = fabricconnection::data_to_transport_msg(data, msg2.put());
    BOOST_REQUIRE(ok);
  }

  const FABRIC_TRANSPORT_MESSAGE_BUFFER *header = {};
  ULONG msgCount = {};
  const FABRIC_TRANSPORT_MESSAGE_BUFFER *body = {};

  msg2->GetHeaderAndBodyBuffer(&header, &msgCount, &body);
  BOOST_CHECK(header != nullptr);
  BOOST_CHECK(msgCount == 1);
  BOOST_CHECK(body != nullptr);

  BOOST_CHECK_EQUAL("myheader",
                    std::string(reinterpret_cast<char *>(header->Buffer),
                                header->BufferSize));
  BOOST_CHECK_EQUAL(
      "mybody",
      std::string(reinterpret_cast<char *>(body->Buffer), body->BufferSize));
}

BOOST_AUTO_TEST_SUITE_END()