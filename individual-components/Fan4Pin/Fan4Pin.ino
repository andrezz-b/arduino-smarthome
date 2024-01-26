#define USE_TIMER_1 true
#define TIMER_INTERVAL_MS 1000L

#include <TimerInterrupt.h>

#define FAN_SENSE_PIN 2
#define FAN_CONTROL_PIN 3
static uint16_t rpm_count = 0;

void incrementRpmCounter() {
  rpm_count++;
}

void print_rpm() {
  noInterrupts();
  // rpm = rpm_count * 30
  uint16_t rpm = (rpm_count << 5) - (rpm_count << 1);
  rpm_count = 0;
  interrupts();
  Serial.print(rpm);
  Serial.println(F(" rpm"));
}

void setup() {
  Serial.begin(115200);
  while (!Serial)
    ;

  pinMode(FAN_CONTROL_PIN, OUTPUT);
  analogWrite(FAN_CONTROL_PIN, 150);

  pinMode(FAN_SENSE_PIN, INPUT_PULLUP);
  attachInterrupt(digitalPinToInterrupt(FAN_SENSE_PIN), incrementRpmCounter, RISING);

  ITimer1.init();

  if (ITimer1.attachInterruptInterval(TIMER_INTERVAL_MS, print_rpm)) {
    Serial.println("Starting  ITimer1 OK, millis() = " + String(millis()));
  } else {
    Serial.println("Can't set ITimer1. Select another freq. or timer");
  }
}

void loop() {
}
