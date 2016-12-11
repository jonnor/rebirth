
#include "color.hpp"
#include "averager.hpp"
#include "parser.hpp"

struct Pins {
  const static int LedR = 9;
  const static int LedG = 10;
  const static int LedB = 11;
  const static int DistanceSensor = 0; // analog in
};

// From https://github.com/guillaume-rico/SharpIR/blob/master/SharpIR.cpp#L97
int calculateDistanceCm(int value) {
  // for Sharp 2Y0A02
  return 60.374 * pow(map(value, 0, 1023, 0, 5000)/1000.0, -1.16);
}

// Arduino code
Parser parser;
long nextReportTime = 0;
static const int reportIntervalMs = 100;
static const int averagingSamples = 20;
Averager<averagingSamples> averager;

void setup() {
  pinMode(Pins::LedR, OUTPUT);
  pinMode(Pins::LedG, OUTPUT);
  pinMode(Pins::LedB, OUTPUT);
  pinMode(Pins::DistanceSensor, INPUT);
  Serial.begin(9600);
  while (!Serial) {
    ; // wait for serial port to connect. Needed for native USB
  }
  Serial.write("ready\n");
  parser.color = { 33, 0, 0 };
  const auto &c = parser.color;
  analogWrite(Pins::LedR, c.r);
  analogWrite(Pins::LedG, c.g);
  analogWrite(Pins::LedB, c.b);
}

void loop() {
  
  // Check for serial input to change LEDs
  if (Serial.available() > 0) {
    const uint8_t in = Serial.read();
    const bool changed = parser.push(in);
    if (changed) {
      auto &c = parser.color;
      analogWrite(Pins::LedR, c.r);
      analogWrite(Pins::LedG, c.g);
      analogWrite(Pins::LedB, c.b);
      static const int BUF_MAX = 50;
      char buf[BUF_MAX] = {0};
      snprintf(buf, BUF_MAX, "updated r=%d g=%d b=%d\n", c.r, c.g, c.b);
      Serial.write(buf);
    }
  }
  
  // Read the sensor values
  const int val = analogRead(Pins::DistanceSensor);
  averager.push(val);

  // Report sensor value
  const long currentTime = millis();
  if (currentTime >= nextReportTime) {
    static const int BUF_MAX = 50;
    char buf[BUF_MAX] = {0};
    const int distance = calculateDistanceCm(averager.getMedian());
    snprintf(buf, BUF_MAX, "updated distance=%d\n", distance);
    Serial.write(buf);
    nextReportTime = currentTime+reportIntervalMs;
  }
}
