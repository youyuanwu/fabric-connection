#include "listener.hpp"
#include "namedpipe_server.hpp"

#include "servicefabric/async_context.hpp"
#include "servicefabric/string_result.hpp"

#include <cassert>

namespace fabricconnection {

namespace sf = servicefabric;

Listener::Listener() {}

void Listener::Initialize(
    /* [in] */ std::string addr,
    /* [in] */ IFabricTransportMessageHandler *requestHandler,
    /* [in] */ IFabricTransportConnectionHandler *connectionHandler) {
  assert(requestHandler != nullptr);
  assert(connectionHandler != nullptr);
  addr_ = addr;
  requestHandler->AddRef();
  handler_.Attach(requestHandler);
  connectionHandler->AddRef();
  connHandler_.Attach(connectionHandler);

  listener_ = std::make_unique<listener_internal>(addr, requestHandler,
                                                  connectionHandler);
  assert(listener_ != nullptr);
}

HRESULT STDMETHODCALLTYPE Listener::BeginOpen(
    /* [in] */ IFabricAsyncOperationCallback *callback,
    /* [retval][out] */ IFabricAsyncOperationContext **context) {

  listener_->run_async();
  belt::com::com_ptr<IFabricAsyncOperationContext> ctx =
      sf::async_context::create_instance(callback).to_ptr();

  *context = ctx.detach();
  return S_OK;
}

HRESULT STDMETHODCALLTYPE Listener::EndOpen(
    /* [in] */ IFabricAsyncOperationContext *context,
    /* [retval][out] */ IFabricStringResult **serviceAddress) {
  UNREFERENCED_PARAMETER(context);
  std::wstring waddr(addr_.begin(), addr_.end());
  belt::com::com_ptr<IFabricStringResult> str =
      sf::string_result::create_instance(waddr).to_ptr();
  *serviceAddress = str.detach();
  return S_OK;
}

HRESULT STDMETHODCALLTYPE Listener::BeginClose(
    /* [in] */ IFabricAsyncOperationCallback *callback,
    /* [retval][out] */ IFabricAsyncOperationContext **context) {

  listener_->stop();
  belt::com::com_ptr<IFabricAsyncOperationContext> ctx =
      sf::async_context::create_instance(callback).to_ptr();

  *context = ctx.detach();
  return S_OK;
}

HRESULT STDMETHODCALLTYPE Listener::EndClose(
    /* [in] */ IFabricAsyncOperationContext *context) {
  UNREFERENCED_PARAMETER(context);
  return S_OK;
}

void STDMETHODCALLTYPE Listener::Abort(void) {}

} // namespace fabricconnection