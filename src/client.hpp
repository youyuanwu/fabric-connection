#pragma once

#include <atlbase.h>
#include <atlcom.h>

// #include "listenerinternal.hpp"
#include <fabrictransport_.h>

#include <string>

namespace fabricconnection {

class Client : public CComObjectRootEx<CComMultiThreadModel>,
               public IFabricTransportClient {
  BEGIN_COM_MAP(Client)
  COM_INTERFACE_ENTRY(IFabricTransportClient)
  END_COM_MAP()

public:
  Client();

  void Init(IFabricTransportCallbackMessageHandler *oneway_handler,
            IFabricTransportClientEventHandler *event_handler,
            std::string addr);

  HRESULT STDMETHODCALLTYPE BeginRequest(
      /* [in] */ IFabricTransportMessage *message,
      /* [in] */ DWORD timeoutMilliseconds,
      /* [in] */ IFabricAsyncOperationCallback *callback,
      /* [retval][out] */ IFabricAsyncOperationContext **context) override;

  HRESULT STDMETHODCALLTYPE EndRequest(
      /* [in] */ IFabricAsyncOperationContext *context,
      /* [retval][out] */ IFabricTransportMessage **reply) override;

  HRESULT STDMETHODCALLTYPE Send(
      /* [in] */ IFabricTransportMessage *message) override;

  HRESULT STDMETHODCALLTYPE BeginOpen(
      /* [in] */ DWORD timeoutMilliseconds,
      /* [in] */ IFabricAsyncOperationCallback *callback,
      /* [retval][out] */ IFabricAsyncOperationContext **context) override;

  HRESULT STDMETHODCALLTYPE EndOpen(
      /* [in] */ IFabricAsyncOperationContext *context) override;

  HRESULT STDMETHODCALLTYPE BeginClose(
      /* [in] */ DWORD timeoutMilliseconds,
      /* [in] */ IFabricAsyncOperationCallback *callback,
      /* [retval][out] */ IFabricAsyncOperationContext **context) override;

  HRESULT STDMETHODCALLTYPE EndClose(
      /* [in] */ IFabricAsyncOperationContext *context) override;

  void STDMETHODCALLTYPE Abort(void) override;

private:
  CComPtr<IFabricTransportCallbackMessageHandler> oneway_handler_;
  CComPtr<IFabricTransportClientEventHandler> event_handler_;
  std::string addr_;
};

} // namespace fabricconnection