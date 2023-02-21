#include <atlbase.h>
#include <atlcom.h>

#include "listenerinternal.hpp"
#include <fabrictransport_.h>

#include <memory>
#include <string>

namespace fabricconnection {

class Listener : public CComObjectRootEx<CComMultiThreadModel>,
                 public IFabricTransportListener {
  BEGIN_COM_MAP(Listener)
  COM_INTERFACE_ENTRY(IFabricTransportListener)
  END_COM_MAP()

public:
  Listener();

  void
  Initialize(/* [in] */ std::string addr,
             /* [in] */ IFabricTransportMessageHandler *requestHandler,
             /* [in] */ IFabricTransportConnectionHandler *connectionHandler);

  HRESULT STDMETHODCALLTYPE BeginOpen(
      /* [in] */ IFabricAsyncOperationCallback *callback,
      /* [retval][out] */ IFabricAsyncOperationContext **context) override;

  HRESULT STDMETHODCALLTYPE EndOpen(
      /* [in] */ IFabricAsyncOperationContext *context,
      /* [retval][out] */ IFabricStringResult **serviceAddress) override;

  HRESULT STDMETHODCALLTYPE BeginClose(
      /* [in] */ IFabricAsyncOperationCallback *callback,
      /* [retval][out] */ IFabricAsyncOperationContext **context) override;

  HRESULT STDMETHODCALLTYPE EndClose(
      /* [in] */ IFabricAsyncOperationContext *context) override;

  void STDMETHODCALLTYPE Abort(void) override;

private:
  CComPtr<IFabricTransportMessageHandler> handler_;
  CComPtr<IFabricTransportConnectionHandler> connHandler_;
  std::unique_ptr<listener_internal> listener_;
  std::string addr_;
};

} // namespace fabricconnection