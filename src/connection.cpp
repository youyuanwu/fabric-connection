#include "fabricconnection/connection.hpp"
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

} // namespace fabricconnection