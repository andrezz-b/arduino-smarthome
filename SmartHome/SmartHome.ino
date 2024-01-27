// Web server Config
#include "SimpleWebServer.h"
#include <UnoWiFiDevEdSerial1.h>
#include "WiFiEspAT.h"
#define EspSerial Serial1
WiFiWebServer server;
const char TEXT_PLAIN[] = "text/plain";

// Temperature sensor config
#include <OneWire.h>
#include <DallasTemperature.h>
#define TMP_SENSOR_PIN 4
OneWire oneWire(TMP_SENSOR_PIN);
DallasTemperature sensors(&oneWire);
DeviceAddress tempSensor;

// IRremote config
#include "TvCommands.h"
#define IR_SEND_PIN 5
#include <IRremote.hpp>
#if defined (USE_SAMSUNG)
#define SendTvCommand(addr, cmd, repeat) IrSender.sendSamsung48(addr, cmd, repeat)
#else if defined (USE_PANASONIC) 
#define SendTvCommand(addr, cmd, repeat) IrSender.sendPanasonic(addr, cmd, repeat) 
#endif
// Fan config
#define FAN_SENSE_PIN 2
#define FAN_CONTROL_PIN 3
#define FAN_LEVELS 5
volatile uint16_t pulse_count = 0;
volatile uint16_t rpm = 0;
volatile uint8_t current_fan_level = 0;
enum FanMode { FanAuto,
               FanManual,
               FanInvalid };
const uint8_t fan_levels[] = { 0, 50, 100, 150, 200 };
FanMode current_mode = FanMode::FanAuto;

//Led
#define LED_PIN LED_BUILTIN
uint8_t led_status = LOW;

/**
 * @brief Initializes the necessary components and configurations for the setup of the SmartHome system.
 * 
 * This function sets up the serial communication, initializes the LED pin, begins the temperature sensor,
 * finds the address for the temperature sensor device, initializes the IR sender, and initializes the fan and WiFi.
 */
void setup() {
  Serial.begin(115200);
  while (!Serial)
    ;

  pinMode(LED_PIN, OUTPUT);
  digitalWrite(LED_PIN, led_status);
  sensors.begin();
  if (!sensors.getAddress(tempSensor, 0)) Serial.println(F("Unable to find address for Device 0"));
  IrSender.begin(false);
  initFan();
  initWiFi();
}

/**
 * @brief The main loop function that runs repeatedly.
 *        It handles client requests and updates the fan level in auto mode.
 */
void loop() {
  server.handleClient();
  fan_level_auto_update();
}

// Led
void get_led_status() {
  server.send(200, TEXT_PLAIN, String(led_status).c_str());
}

/**
 * Changes the status of the LED based on the provided status value.
 * The status value is specified using the `status` query parameter.
 */
void change_led_status() {
  const String status = server.arg(F("status"));
  if (status == "0") {
    led_status = LOW;
  } else if (status == "1") {
    led_status = HIGH;
  } else {
    return server.send(400, TEXT_PLAIN, String(F("invalid_status")).c_str());
  }
  digitalWrite(LED_PIN, led_status);
  server.send(200, TEXT_PLAIN, String(F("success")).c_str());
}

// Temperature
/**
 * @brief Get the temperature in Celsius.
 * This function requests the temperature from the sensors and returns the temperature in Celsius.
 * @return The temperature in Celsius.
 */
float get_temperature_C() {
  sensors.requestTemperatures();
  return sensors.getTempC(tempSensor);
}

void read_temp() {
  server.send(200, TEXT_PLAIN, String(get_temperature_C()).c_str());
}

// Remote
/**
 * Sends a TV remote command to the IR sender.
 * The command is specified using the `command` query parameter.
 */
void send_remote_command() {
  const String command = server.arg(F("command"));
  if (!command || !sendCommand(command)) {
    server.send(400, TEXT_PLAIN, "invalid_command");
  }
  server.send(200, TEXT_PLAIN, "success");
}

/**
 * Sends a command to the IR sender based on the given command string.
 * 
 * @param command The command string to be sent.
 * @return True if the command was sent successfully, false otherwise.
 */
bool sendCommand(const String& command) {
  if (command == "power") {
    SendTvCommand(ADDR_POWER, CMD_POWER, 2);
    delay(5);
    SendTvCommand(ADDR_POWER, CMD_POWER, 2);
    return true;
  } else if (command == "volume-up") {
    SendTvCommand(ADDR_VOLUME_DOWN, CMD_VOLUME_DOWN, 0);
    return true;
  } else if (command == "volume-down") {
    SendTvCommand(ADDR_VOLUME_UP, CMD_VOLUME_UP, 0);
    return true;
  } else if (command == "channel-up") {
    SendTvCommand(ADDR_CHANNEL_FORWARD, CMD_CHANNEL_FORWARD, 0);
    return true;
  } else if (command == "channel-down") {
    SendTvCommand(ADDR_CHANNEL_BACK, CMD_CHANNEL_BACK, 0);
    return true;
  } else if (command == "mute") {
    SendTvCommand(ADDR_MUTE, CMD_MUTE, 0);
    return true;
  } else if (command == "return") {
    SendTvCommand(ADDR_RETURN, CMD_RETURN, 0);
    return true;
  }

  int8_t command_int = (int8_t)command.toInt();
  if (command == "0") {
    SendTvCommand(ADDR_NUMBER, CMD_ZERO, 0);
    return true;
  } else if (command_int >= 1 && command_int <= 9) {
    #if defined(USE_SAMSUNG)
    // Adjust the command to match the Samsung remote
    uint8_t offset = 0;
    offset = command_int >= 4 ? 1 : offset;
    offset = command_int >= 7 ? 2 : offset;
    command_int += offset;
    #endif
    SendTvCommand(ADDR_NUMBER, CMD_ONE + command_int - 1, 0);
    return true;
  }

  return false;
}
// Fan
/**
 * Retrieves the status of the fan, including the current RPM, fan level, and mode.
 * Sends a HTTP response which is a string containing the current RPM, fan level, and mode, separated by commas.
 */
void get_fan_status() {
  noInterrupts();
  uint16_t current_rpm = rpm;
  interrupts();
  const String res = String(current_rpm) + "," + String(current_fan_level) + "," + String(current_mode);
  server.send(200, TEXT_PLAIN, res.c_str());
}

void change_fan_mode() {
  const String mode = server.arg(F("mode"));
  FanMode parsed_mode = parse_fan_mode(mode);
  if (parsed_mode == FanMode::FanInvalid) {
    return server.send(400, TEXT_PLAIN, String(F("invalid_mode")).c_str());
  }
  current_mode = parsed_mode;
  server.send(200, TEXT_PLAIN, String(F("mode_changed")).c_str());
}


/**
 * Parses the given fan mode string and returns the corresponding FanMode value.
 * 
 * @param mode The fan mode string to parse.
 * @return The FanMode value corresponding to the parsed fan mode string.
 */
FanMode parse_fan_mode(const String& mode) {
  if (mode == String(F("auto"))) {
    return FanMode::FanAuto;
  } else if (mode == String(F("manual"))) {
    return FanMode::FanManual;
  }

  return FanMode::FanInvalid;
}

/**
 * Changes the fan level based on the provided `level` query parameter.
 */
void change_fan_level() {
  const String level = server.arg(F("level"));
  int8_t parsed_level = parse_fan_level(level);
  if (parsed_level == -1) {
    return server.send(400, TEXT_PLAIN, String(F("invalid_level")).c_str());
  }
  current_mode = FanMode::FanManual;
  current_fan_level = (uint8_t)parsed_level;
  analogWrite(FAN_CONTROL_PIN, fan_levels[current_fan_level]);
  server.send(200, TEXT_PLAIN, String(F("level_changed")).c_str());
}

/**
 * Parses the fan level command and returns the corresponding fan level.
 * 
 * @param level The fan level command to parse.
 * @return The parsed fan level. Returns -1 if the command is invalid.
 */
int8_t parse_fan_level(const String& level) {
  if (level == String(F("inc"))) {
    return current_fan_level >= FAN_LEVELS - 1 ? FAN_LEVELS - 1 : current_fan_level + 1;
  } else if (level == String(F("dec"))) {
    return !current_fan_level ? 0 : current_fan_level - 1;
  } else if (level == String(F("min"))) {
    return 0;
  } else if (level == String(F("max"))) {
    return (int8_t)sizeof(fan_levels) - 1;
  }

  return -1;
}

/**
 * Updates the fan level based on the current temperature.
 * If the current mode is FanManual, the function returns without making any changes.
 * The fan level is determined based on the temperature reading in Celsius.
 * If the temperature is below 18°C, the fan level is set to 0.
 * If the temperature is between 18°C and 20.9°C, the fan level is set to 1.
 * If the temperature is between 21°C and 23.9°C, the fan level is set to 2.
 * If the temperature is between 24°C and 26.9°C, the fan level is set to 3.
 * If the temperature is 27°C or above, the fan level is set to 4.
 * Finally, the fan control pin is updated using the corresponding fan level value.
 */
void fan_level_auto_update() {
  if (current_mode == FanMode::FanManual) return;
  const float temp = get_temperature_C();
  if (temp < 18) {
    current_fan_level = 0;
  } else if (temp < 21) {
    current_fan_level = 1;
  } else if (temp < 24) {
    current_fan_level = 2;
  } else if (temp < 27) {
    current_fan_level = 3;
  } else {
    current_fan_level = 4;
  }

  analogWrite(FAN_CONTROL_PIN, fan_levels[current_fan_level]);
}

void increment_pulse_counter() {
  pulse_count++;
}

/**
 * @brief Interrupt Service Routine for TIMER1_COMPA_vect.
 * This ISR calculates the RPM based on the pulse count.
 * It resets the pulse count to zero after calculating the RPM.
 */
ISR(TIMER1_COMPA_vect) {
  // rpm = pulse_count * 30
  rpm = (pulse_count << 5) - (pulse_count << 1);
  pulse_count = 0;
}

/**
 * Initializes the fan control and RPM calculation.
 * Sets the pin modes, attaches an interrupt, and configures the timer for RPM calculation.
 */
void initFan() {
  pinMode(FAN_CONTROL_PIN, OUTPUT);
  analogWrite(FAN_CONTROL_PIN, 0);

  pinMode(FAN_SENSE_PIN, INPUT_PULLUP);
  attachInterrupt(digitalPinToInterrupt(FAN_SENSE_PIN), increment_pulse_counter, RISING);

  // Setup timer1 for RPM calculation
  TCCR1A = 0;
  TCCR1B = (1 << WGM12);
  TIMSK1 = (1 << OCIE1A);
  OCR1A = 62501;
  TCCR1B |= (1 << CS12);
  interrupts();
}

/**
 * Initializes the WiFi shield and connects to the WiFi network.
 * This function also sets up the server routes for handling HTTP requests.
 */
void initWiFi() {

  EspSerial.begin(115200);
  WiFi.init(&EspSerial);
  Serial.println(F("WiFi shield init done"));

  if (WiFi.status() == WL_NO_MODULE) {
    Serial.println(F("Communication with WiFi module failed!"));
    while (true)
      ;
  }

  Serial.println(F("Waiting for connection to WiFi"));
  while (WiFi.status() != WL_CONNECTED) {
    delay(1000);
    Serial.print('.');
  }
  Serial.println();

  Serial.print(F("You're connected to the network, IP = "));
  Serial.print(WiFi.localIP());
  Serial.print(F(" SSID: "));
  Serial.println(WiFi.SSID());
  Serial.println();

  server.on(F("/temp"), HTTP_GET, read_temp);
  server.on(F("/remote"), HTTP_GET, send_remote_command);
  server.on(F("/fan/status"), HTTP_GET, get_fan_status);
  server.on(F("/fan/mode"), HTTP_GET, change_fan_mode);
  server.on(F("/fan/level"), HTTP_GET, change_fan_level);
  server.on(F("/led/status"), HTTP_GET, get_led_status);
  server.on(F("/led/change"), HTTP_GET, change_led_status);

  server.begin();
}