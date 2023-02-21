#include "fabricconnection/connection.hpp"
#include "client.hpp"
#include "listener.hpp"
#include <cassert>

#include <atlbase.h>
#include <atlcom.h>

namespace fabricconnection {

/* [entry] */ HRESULT CreateFabricConnectionListener(
    /* [in] */ FabricConnectionSettings *settings,
    /* [in] */ IFabricTransportMessageHandler *requestHandler,
    /* [in] */ IFabricTransportConnectionHandler *connectionHandler,
    /* [retval][out] */ IFabricTransportListener **listener) {
  assert(settings != nullptr);
  assert(requestHandler != nullptr);
  assert(connectionHandler != nullptr);
  assert(listener != nullptr);

  CComPtr<CComObjectNoLock<Listener>> ret(new CComObjectNoLock<Listener>());
  ret->Initialize(settings->Address, requestHandler, connectionHandler);
  *listener = ret.Detach();
  return S_OK;
}

/* [entry] */ HRESULT CreateFabricTransportClient(
    /* [in] */ FabricConnectionSettings *settings,
    /* [in] */ IFabricTransportCallbackMessageHandler *notificationHandler,
    /* [in] */ IFabricTransportClientEventHandler *clientEventHandler,
    /* [retval][out] */ IFabricTransportClient **client) {
  assert(settings != nullptr);

  CComPtr<CComObjectNoLock<Client>> ret(new CComObjectNoLock<Client>());
  ret->Init(notificationHandler, clientEventHandler, settings->Address);
  *client = ret.Detach();
  return S_OK;
}

} // namespace fabricconnection