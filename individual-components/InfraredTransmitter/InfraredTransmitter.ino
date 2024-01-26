/****************************************************************************************************************************
  WebServer.ino - Simple Arduino web server sample for SAMD21 running WiFiNINA shield
  For any WiFi shields, such as WiFiNINA W101, W102, W13x, or custom, such as ESP8266/ESP32-AT, Ethernet, etc

  WiFiWebServer is a library for the ESP32-based WiFi shields to run WebServer
  Based on and modified from ESP8266 https://github.com/esp8266/Arduino/releases
  Based on  and modified from Arduino WiFiNINA library https://www.arduino.cc/en/Reference/WiFiNINA
  Built by Khoi Hoang https://github.com/khoih-prog/WiFiWebServer
  Licensed under MIT license

  A simple web server that shows the value of the analog input pins via a web page using an ESP8266 module.
  This sketch will start an access point and print the IP address of your ESP8266 module to the Serial monitor.
  From there, you can open that address in a web browser to display the web page.
  The web page will be automatically refreshed each 20 seconds.

  For more details see: http://yaab-arduino.blogspot.com/p/wifiesp.html
 ***************************************************************************************************************************************/
#include "defines.h"
#include <IRremote.hpp>       // include the library
int status = WL_IDLE_STATUS;  // the Wifi radio's status
int reqCount = 0;             // number of requests received

WiFiServer server(80);

void printWifiStatus() {
}

void setup() {
  Serial.begin(115200);
  while (!Serial)
    ;
#if WIFI_USING_ESP_AT

  // initialize serial for ESP module
  EspSerial.begin(115200);
  // initialize ESP module
  WiFi.init(&EspSerial);

  Serial.println(F("WiFi shield init done"));

#endif

  if (WiFi.status() == WL_NO_MODULE) {
    Serial.println("Communication with WiFi module failed!");
    while (true)
      ;
  }

  Serial.println("Waiting for connection to WiFi");
  while (WiFi.status() != WL_CONNECTED) {
    delay(1000);
    Serial.print('.');
  }
  Serial.println();

  Serial.print(F("You're connected to the network, IP = "));
  Serial.println(WiFi.localIP());

  Serial.print(F("SSID: "));
  Serial.println(WiFi.SSID());

  server.begin();

  pinMode(LED_BUILTIN, OUTPUT);
  IrSender.begin();
}

void loop() {
  // listen for incoming clients
  WiFiClient client = server.available();
  if (client) {
    processClient(client);
  }
}

void processClient(WiFiClient &client) {
  Serial.println(F("New client"));
  while (client.connected()) {
    if (!client.available()) continue;

    if (handleRequest(client)) break;
  }

  // give the web browser time to receive the data
  delay(10);

  // close the connection:
  client.stop();
  Serial.println(F("Client disconnected"));
}

void handleNotFound(WiFiClient &client) {
  client.print("HTTP/1.1 404 Not Found\r\n"
               "Content-Length: 47\r\n"
               "Content-Type: text/plain\r\n"
               "Connection: Closed\r\n"
               "\r\n"
               "Invalid url, please try GET /remote/<command>\r\n");
}

bool handleRequest(WiFiClient &client) {
  String req = client.readStringUntil('\r');
  client.readStringUntil('\n');

  int8_t addr_start = req.indexOf(' ');
  int8_t addr_end = req.indexOf(' ', addr_start + 1);

  String methodStr = req.substring(0, addr_start);
  while (client.available()) {
    client.read();
  }
  if (methodStr != "GET") {
    handleNotFound(client);
    return true;
  }
  String url = req.substring(addr_start + 1, addr_end);
  if (url.charAt(url.length() - 1) == '/') {
    url = url.substring(0, url.length() - 1);
  }
  int8_t pos = url.indexOf("remote");
  if (pos == -1) {
    handleNotFound(client);
    return true;
  }
  String argument = getArgument(url);
  if (argument.length() == 0) {
    handleNotFound(client);
    return true;
  }

  if (!sendCommand(argument)) {
    handleNotFound(client);
    return true;
  }

  client.print("HTTP/1.1 200 OK\r\n"
               "Content-Type: text/plain\r\n"
               "Connection: Closed\r\n"
               "\r\n"
               "Command ");
  client.println(argument);

  return true;
}

String getArgument(const String &url) {
  int8_t slashIndex = url.lastIndexOf('/');
  if (slashIndex == -1 || slashIndex == 0) {
    return "";
  }
  return url.substring(slashIndex + 1);
}

bool sendCommand(const String &command) {
  if (command == "power") {
    IrSender.sendPanasonic(ADDR_POWER, CMD_POWER, 0);
    return true;
  } else if (command == "volume-up") {
    IrSender.sendPanasonic(ADDR_VOLUME_DOWN, CMD_VOLUME_DOWN, 0);
    return true;
  } else if (command == "volume-down") {
    IrSender.sendPanasonic(ADDR_VOLUME_UP, CMD_VOLUME_UP, 0);
    return true;
  }

  return false;
}
