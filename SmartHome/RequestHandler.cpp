#include "RequestHandler.h"

FunctionRequestHandler::FunctionRequestHandler(WiFiWebServer::THandlerFunction fn, const String& uri,
                                               const HTTPMethod& method)
  : _fn(fn), _uri(uri), _method(method) {
}
bool FunctionRequestHandler::canHandle(const HTTPMethod& requestMethod, const String& requestUri) {
  if (_method != HTTP_ANY && _method != requestMethod)
    return false;

  if (requestUri == _uri)
    return true;

  if (_uri.endsWith("/*")) {
    String _uristart = _uri;
    _uristart.replace("/*", "");

    if (requestUri.startsWith(_uristart))
      return true;
  }

  return false;
}

bool FunctionRequestHandler::handle(WiFiWebServer& server, const HTTPMethod& requestMethod, /*const*/ String& requestUri) {
  WFW_UNUSED(server);

  if (!canHandle(requestMethod, requestUri))
    return false;

  _fn();
  return true;
}
