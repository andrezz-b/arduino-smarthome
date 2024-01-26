#include "WString.h"
#ifndef SimpleWebServer_h
#define SimpleWebServer_h

#include <Arduino.h>
#include <stdint.h>
#include "WiFiEspAT.h"

#define RETURN_NEWLINE "\r\n"

enum HTTPClientStatus {
  HC_NONE,
  HC_WAIT_READ,
  HC_WAIT_CLOSE
};

enum HTTPMethod {
  HTTP_ANY,
  HTTP_GET,
  HTTP_HEAD,
  HTTP_POST,
  HTTP_PUT,
  HTTP_PATCH,
  HTTP_DELETE,
  HTTP_OPTIONS
};

class RequestHandler;

#define HTTP_MAX_DATA_WAIT 5000   //ms to wait for the client to send the request
#define HTTP_MAX_CLOSE_WAIT 2000  //ms to wait for the client to close the connection
#define HTTP_MAX_SEND_WAIT 5000   //ms to wait for data chunk to be ACKed

class WiFiWebServer {
public:
  typedef void (*THandlerFunction)(void);
  WiFiWebServer(uint16_t port = 80);
  ~WiFiWebServer();

  void begin();
  void handleClient();

  void on(const String& uri, THandlerFunction handler);
  void on(const String& uri, HTTPMethod method, THandlerFunction fn);

  String arg(const String& name);  // get request argument value by name

  void send(int code, const char* content_type, const char* content);
  void send(int code, const char* content_type, const char* content, size_t contentLength);
private:

  void _prepareHeader(String &response, int code, const char* content_type, size_t contentLength);
  String _responseCodeToString(int code);

  bool parseRequest(WiFiClient& client);
  void parseArguments(const String& data);
  String WiFiWebServer::urlDecode(const String& text);

  WiFiServer server;
  WiFiClient currentClient;
  HTTPClientStatus clientStatus;

  HTTPMethod currentMethod;
  String currentUri;

  unsigned long statusChange;

  // Handler
  RequestHandler* _currentHandler = nullptr;
  RequestHandler* _firstHandler = nullptr;
  RequestHandler* _lastHandler = nullptr;
  THandlerFunction _notFoundHandler;
  void _addRequestHandler(RequestHandler* handler);
  void handleRequest();

  // Arguments
  struct RequestArgument {
    String key;
    String value;
  };
  uint8_t currentArgCount;
  RequestArgument* currentArgs = nullptr;
};

#endif  //SimpleWebServer_h
