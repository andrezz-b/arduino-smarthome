#include "HardwareSerial.h"
#include "WString.h"
#include "Arduino.h"
#include "SimpleWebServer.h"
#include "RequestHandler.h"
extern unsigned int __bss_end;
extern void* __brkval;

WiFiWebServer::WiFiWebServer(uint16_t port = 80)
  : server(port), clientStatus(HC_NONE), statusChange(0), currentMethod(HTTP_ANY), currentArgCount(0), currentArgs(nullptr), _currentHandler(nullptr), _firstHandler(nullptr), _lastHandler(nullptr) {
}

WiFiWebServer::~WiFiWebServer() {
}

void WiFiWebServer::begin() {
  server.begin();
}

void WiFiWebServer::handleClient() {
  if (clientStatus == HC_NONE) {
    WiFiClient client = server.available();
    if (!client) {
      delay(1);
      return;
    }
    currentClient = client;
    clientStatus = HC_WAIT_READ;
    statusChange = millis();
  }

  bool keepCurrentClient = false;
  bool callYield = false;

  if (currentClient.connected() || currentClient.available()) {
    switch (clientStatus) {
      case HC_NONE:
        break;

      case HC_WAIT_READ:

        // Wait for data from client to become available
        if (currentClient.available()) {
          if (parseRequest(currentClient)) {
            currentClient.setTimeout(HTTP_MAX_SEND_WAIT);
            handleRequest();
          }
        } else {
          if (millis() - statusChange <= HTTP_MAX_DATA_WAIT) {
            keepCurrentClient = true;
          }
          callYield = true;
        }
        break;

      case HC_WAIT_CLOSE:
        // Wait for client to close the connection
        if (millis() - statusChange <= HTTP_MAX_CLOSE_WAIT) {
          keepCurrentClient = true;
          callYield = true;
        }
    }
  }

  if (!keepCurrentClient) {
    currentClient.stop();
    clientStatus = HC_NONE;
  }

  if (callYield) {
    yield();
  }
}

void WiFiWebServer::handleRequest() {
  bool handled = false;

  if (!_currentHandler) {
  } else {
    handled = _currentHandler->handle(*this, currentMethod, currentUri);
  }

  if (!handled && _notFoundHandler) {

    _notFoundHandler();
    handled = true;
  }

  if (!handled && currentMethod == HTTP_OPTIONS) {
    send(200, "text/plain", "");
    handled = true;
  }

  if (!handled) {
    send(404, "text/html", currentUri.c_str());
    handled = true;
  }

  if (handled) {
  }
}

void WiFiWebServer::on(const String& uri, WiFiWebServer::THandlerFunction handler) {
  on(uri, HTTP_ANY, handler);
}

void WiFiWebServer::on(const String& uri, HTTPMethod method, WiFiWebServer::THandlerFunction fn) {
  _addRequestHandler(new FunctionRequestHandler(fn, uri, method));
}

void WiFiWebServer::_addRequestHandler(RequestHandler* handler) {
  if (!_lastHandler) {
    _firstHandler = handler;
    _lastHandler = handler;
  } else {
    _lastHandler->next(handler);
    _lastHandler = handler;
  }
}

String WiFiWebServer::arg(const String& name) {
  for (int i = 0; i < currentArgCount; ++i) {
    if (currentArgs[i].key == name)
      return currentArgs[i].value;
  }

  return String();
}

// Parsing
bool WiFiWebServer::parseRequest(WiFiClient& client) {
  // Read the first line of HTTP request
  String req = client.readStringUntil('\r');
  client.readStringUntil('\n');

  // First line of HTTP request looks like "GET /path HTTP/1.1"
  // Retrieve the "/path" part by finding the spaces
  int addr_start = req.indexOf(' ');
  int addr_end = req.indexOf(' ', addr_start + 1);

  if (addr_start == -1 || addr_end == -1) {
    return false;
  }

  String methodStr = req.substring(0, addr_start);
  String url = req.substring(addr_start + 1, addr_end);
  String searchStr = "";
  int hasSearch = url.indexOf('?');

  if (hasSearch != -1) {
    searchStr = url.substring(hasSearch + 1);
    url = url.substring(0, hasSearch);
  }

  currentUri = url;

  HTTPMethod method = HTTP_GET;

  if (methodStr == "HEAD") {
    method = HTTP_HEAD;
  } else if (methodStr == "POST") {
    method = HTTP_POST;
  } else if (methodStr == "DELETE") {
    method = HTTP_DELETE;
  } else if (methodStr == "OPTIONS") {
    method = HTTP_OPTIONS;
  } else if (methodStr == "PUT") {
    method = HTTP_PUT;
  } else if (methodStr == "PATCH") {
    method = HTTP_PATCH;
  }

  currentMethod = method;

  RequestHandler* handler = nullptr;

  for (handler = _firstHandler; handler; handler = handler->next()) {
    if (handler->canHandle(currentMethod, currentUri))
      break;
  }

  _currentHandler = handler;

  parseArguments(searchStr);

  return true;
}

void WiFiWebServer::parseArguments(const String& data) {
  if (currentArgs)
    delete[] currentArgs;

  currentArgs = nullptr;

  if (data.length() == 0) {
    currentArgCount = 0;
    currentArgs = new RequestArgument[1];

    return;
  }

  currentArgCount = 1;

  for (int i = 0; i < (int)data.length();) {
    i = data.indexOf('&', i);

    if (i == -1)
      break;

    ++i;
    ++currentArgCount;
  }

  currentArgs = new RequestArgument[currentArgCount + 1];

  int pos = 0;
  int iarg;

  for (iarg = 0; iarg < currentArgCount;) {
    int equal_sign_index = data.indexOf('=', pos);
    int next_arg_index = data.indexOf('&', pos);

    if ((equal_sign_index == -1) || ((equal_sign_index > next_arg_index) && (next_arg_index != -1))) {
      if (next_arg_index == -1)
        break;

      pos = next_arg_index + 1;

      continue;
    }

    RequestArgument& arg = currentArgs[iarg];
    arg.key = urlDecode(data.substring(pos, equal_sign_index));
    arg.value = urlDecode(data.substring(equal_sign_index + 1, next_arg_index));
    // arg.key = data.substring(pos, equal_sign_index);
    // arg.value = data.substring(equal_sign_index + 1, next_arg_index);

    ++iarg;

    if (next_arg_index == -1)
      break;

    pos = next_arg_index + 1;
  }

  currentArgCount = iarg;
}

String WiFiWebServer::urlDecode(const String& text) {
  String decoded = "";
  char temp[] = "0x00";
  unsigned int len = text.length();
  unsigned int i = 0;

  while (i < len) {
    char decodedChar;
    char encodedChar = text.charAt(i++);

    if ((encodedChar == '%') && (i + 1 < len)) {
      temp[2] = text.charAt(i++);
      temp[3] = text.charAt(i++);

      decodedChar = strtol(temp, NULL, 16);
    } else {
      if (encodedChar == '+') {
        decodedChar = ' ';
      } else {
        decodedChar = encodedChar;  // normal ascii char
      }
    }

    decoded += decodedChar;
  }

  return decoded;
}

// Send

void WiFiWebServer::send(int code, const char* content_type, const char* content) {
  send(code, content_type, content, content ? strlen(content) : 0);
}

void WiFiWebServer::send(int code, const char* content_type, const char* content, size_t contentLength) {

  String header;
  _prepareHeader(header, code, content_type, contentLength);

  // Serial.write((const uint8_t*)header.c_str(), header.length());
  // currentClient.write((const uint8_t*)header.c_str(), header.length());
  //currentClient.write(header, strlen(header));
  // Serial.write(header, strlen(header));

  if (contentLength) {
    currentClient.write(content, contentLength);
    // Serial.write(content, contentLength);
  }
}

void WiFiWebServer::_prepareHeader(String &response, int code, const char* content_type, size_t contentLength) {

  if (!content_type)
    content_type = "text/html";

  // sprintf(response, "HTTP/1.1 %d %s\r\nContent-Type: %s\r\nContent-Length: %d\r\nConnection: close\r\n\r\n", code, _responseCodeToString(code).c_str(), content_type, contentLength);

  response = "HTTP/1.1 ";
  response += String(code);
  response += " ";
  response += _responseCodeToString(code);
  response += RETURN_NEWLINE;
  currentClient.write(response.c_str(), response.length());

  // if (!content_type)
  //   content_type = "text/html";

  response = F("Content-Type: ");
  response += content_type;
  response += RETURN_NEWLINE;
  currentClient.write(response.c_str(), response.length());

  response = F("Content-Length: ");
  response += String(contentLength);
  response += RETURN_NEWLINE;
   currentClient.write(response.c_str(), response.length());

  response = F("Access-Control-Allow-Origin: *");
  response += RETURN_NEWLINE;
  currentClient.write(response.c_str(), response.length());
  // response = F("Access-Control-Allow-Methods: *");
  // response += RETURN_NEWLINE;
  // currentClient.write(response.c_str(), response.length());

  response = F("Connection: close\r\n");
  response += RETURN_NEWLINE;
  currentClient.write(response.c_str(), response.length());

  response = "";
}

String WiFiWebServer::_responseCodeToString(int code) {
  switch (code) {
    case 200:
      return F("OK");

    case 400:
      return F("Bad Request");

    case 404:
      return F("Not Found");

    case 500:
      return F("Internal Server Error");

    default:
      return "";
  }
}