
#ifndef defines_h
#define defines_h

#include <UnoWiFiDevEdSerial1.h>
#define DEBUG_WIFI_WEBSERVER_PORT Serial

// Debug Level from 0 to 4
#define _WIFI_LOGLEVEL_ 4
#define _WIFININA_LOGLEVEL_ 3


#define USE_WIFI_NINA false
#define USE_WIFI_CUSTOM true

#include "WiFiEspAT.h"
#define WIFI_USING_ESP_AT true
#define EspSerial Serial1


#define SHIELD_TYPE "ESP WiFi using WiFi Library"


#define BOARD_TYPE "AVR Mega"
#define BOARD_NAME "Arduiono UNO WiFi"


char ssid[] = "ISKONOVAC-4035a3-2.4";  // your network SSID (name)
char pass[] = "X77CteTWCABe";   // your network password


// For ATmega328 like on Uno, Nano, Leonardo, Teensy 2.0 etc.
#define IR_RECEIVE_PIN      2 // To be compatible with interrupt example, pin 2 is chosen here.
#define IR_SEND_PIN         3
#define TONE_PIN            4
#define APPLICATION_PIN     5
#define ALTERNATIVE_IR_FEEDBACK_LED_PIN 6 // E.g. used for examples which use LED_BUILDIN for example output.
#define _IR_TIMING_TEST_PIN 7

#if !defined(ARDUINO_SAMD_ADAFRUIT) && !defined(ARDUINO_SEEED_XIAO_M0)
// On the Zero and others we switch explicitly to SerialUSB
// #define Serial SerialUSB
#endif


#if !defined (FLASHEND)
#define FLASHEND 0xFFFF // Dummy value for platforms where FLASHEND is not defined
#endif
#if !defined (RAMEND)
#define RAMEND 0xFFFF // Dummy value for platforms where RAMEND is not defined
#endif
#if !defined (RAMSIZE)
#define RAMSIZE 0xFFFF // Dummy value for platforms where RAMSIZE is not defined
#endif

/*
 * Helper macro for getting a macro definition as string
 */
#if !defined(STR_HELPER)
#define STR_HELPER(x) #x
#define STR(x) STR_HELPER(x)
#endif

// Panasonic Commands
// Power button
#define CMD_POWER 0x3D
#define ADDR_POWER 0x8
// Channel forward
#define CMD_CHANNEL_FORWARD 0x34
#define ADDR_CHANNEL_FORWARD 0x8
// Channel back
#define CMD_CHANNEL_BACK 0x35
#define ADDR_CHANNEL_BACK 0x8
// Volume up
#define CMD_VOLUME_UP 0x20
#define ADDR_VOLUME_UP 0x8
// Volume down
#define CMD_VOLUME_DOWN 0x21
#define ADDR_VOLUME_DOWN 0x8


#endif  //defines_h
