#include <FabricTransport_.h>

#include <string>

namespace fabricconnection {

struct FabricConnectionSettings {
  std::string Address;
};

/* [entry] */ HRESULT CreateFabricConnectionListener(
    /* [in] */ FabricConnectionSettings *settings,
    /* [in] */ IFabricTransportMessageHandler *requestHandler,
    /* [in] */ IFabricTransportConnectionHandler *connectionHandler,
    /* [retval][out] */ IFabricTransportListener **listener);

/* [entry] */ HRESULT CreateFabricTransportClient(
    /* [in] */ FabricConnectionSettings *settings,
    /* [in] */ IFabricTransportCallbackMessageHandler *notificationHandler,
    /* [in] */ IFabricTransportClientEventHandler *clientEventHandler,
    /* [retval][out] */ IFabricTransportClient **client);

} // namespace fabricconnection