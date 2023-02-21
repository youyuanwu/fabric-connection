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

// /* [entry] */ HRESULT CreateFabricTransportClient(
//     /* [in] */ REFIID interfaceId,
//     /* [in] */ FABRIC_TRANSPORT_SETTINGS *settings,
//     /* [in] */ LPCWSTR connectionAddress,
//     /* [in] */ IFabricTransportCallbackMessageHandler *notificationHandler,
//     /* [in] */ IFabricTransportClientEventHandler *clientEventHandler,
//     /* [in] */ IFabricTransportMessageDisposer *messageDisposer,
//     /* [retval][out] */ IFabricTransportClient **client);

} // namespace fabricconnection