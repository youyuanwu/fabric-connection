#include "client.hpp"

#include <cassert>

namespace fabricconnection {

Client::Client() : oneway_handler_(), event_handler_() {}

void Client::Init(IFabricTransportCallbackMessageHandler *oneway_handler,
                  IFabricTransportClientEventHandler *event_handler,
                  std::string addr) {
  assert(oneway_handler != nullptr);
  assert(event_handler != nullptr);
  oneway_handler->AddRef();
  oneway_handler_.Attach(oneway_handler);
  event_handler->AddRef();
  event_handler_.Attach(event_handler);
  addr_ = addr;
}

HRESULT STDMETHODCALLTYPE Client::BeginRequest(
    /* [in] */ IFabricTransportMessage *message,
    /* [in] */ DWORD timeoutMilliseconds,
    /* [in] */ IFabricAsyncOperationCallback *callback,
    /* [retval][out] */ IFabricAsyncOperationContext **context) {
  return S_OK;
}

HRESULT STDMETHODCALLTYPE Client::EndRequest(
    /* [in] */ IFabricAsyncOperationContext *context,
    /* [retval][out] */ IFabricTransportMessage **reply) {
  return S_OK;
}
HRESULT STDMETHODCALLTYPE Client::Send(
    /* [in] */ IFabricTransportMessage *message) {
  return S_OK;
}

HRESULT STDMETHODCALLTYPE Client::BeginOpen(
    /* [in] */ DWORD timeoutMilliseconds,
    /* [in] */ IFabricAsyncOperationCallback *callback,
    /* [retval][out] */ IFabricAsyncOperationContext **context) {
  return S_OK;
}

HRESULT STDMETHODCALLTYPE Client::EndOpen(
    /* [in] */ IFabricAsyncOperationContext *context) {
  return S_OK;
}

HRESULT STDMETHODCALLTYPE Client::BeginClose(
    /* [in] */ DWORD timeoutMilliseconds,
    /* [in] */ IFabricAsyncOperationCallback *callback,
    /* [retval][out] */ IFabricAsyncOperationContext **context) {
  return S_OK;
}

HRESULT STDMETHODCALLTYPE Client::EndClose(
    /* [in] */ IFabricAsyncOperationContext *context) {
  return S_OK;
}

void STDMETHODCALLTYPE Client::Abort(void) {}

} // namespace fabricconnection