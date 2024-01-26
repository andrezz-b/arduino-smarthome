#ifndef RequestHandler_H
#define RequestHandler_H

#ifndef WFW_UNUSED
#define WFW_UNUSED(x) (void)(x)
#endif

#include <Arduino.h>
#include "SimpleWebServer.h"
class RequestHandler {
public:

  virtual ~RequestHandler() {}

  virtual bool canHandle(const HTTPMethod& method, const String& uri) {
    WFW_UNUSED(method);
    WFW_UNUSED(uri);

    return false;
  }

  virtual bool handle(WiFiWebServer& server, const HTTPMethod& requestMethod, /*const*/ String& requestUri) {
    WFW_UNUSED(server);
    WFW_UNUSED(requestMethod);
    WFW_UNUSED(requestUri);

    return false;
  }

  RequestHandler* next() {
    return _next;
  }


  void next(RequestHandler* r) {
    _next = r;
  }

private:

  RequestHandler* _next = nullptr;
};


class FunctionRequestHandler : public RequestHandler {
public:

  FunctionRequestHandler(WiFiWebServer::THandlerFunction fn, const String& uri,
                         const HTTPMethod& method);

  bool canHandle(const HTTPMethod& requestMethod, const String& requestUri) override;

  bool handle(WiFiWebServer& server, const HTTPMethod& requestMethod, /*const*/ String& requestUri) override;

protected:
  WiFiWebServer::THandlerFunction _fn;
  String _uri;
  HTTPMethod _method;
};

#endif  // RequestHandler_H
