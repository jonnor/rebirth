
#include "color.hpp"
#include "animation.hpp"
#include "averager.hpp"

// Configuration
struct Pins {
  const static int LedR = 9;
  const static int LedG = 10;
  const static int LedB = 11;
  const static int DistanceSensor = 0; // analog in
};
// Advanced config, be careful
static const int reportIntervalMs = 300;
static const int animationIntervalMs = 33;
static const int averagingSamples = 20;

// At what distance the sensor will trigger
static const int idleDistanceCm = 120;
Input input = {
    timeMs: 1,
    idle: true,
    // Color when idle, no-one close to sensor
    idleColor: { 0, 255, 0 },
    // Time to complete one breathing cycle (milliseconds)
    breathingPeriodMs: 2500,
    // Color of the breathing at max
    breathingColor: { 200, 200, 255 },
    // How fast the heartbeat is beating (beats per minute)
    heartRate: 133,
    // Color of the heartbeat
    heartbeatColor: { 200, 30, 30 },
    // Duration of one heartbeat
    heartbeatLengthMs: 80,
};

// From https://github.com/guillaume-rico/SharpIR/blob/master/SharpIR.cpp#L97
int calculateDistanceCm(int value) {
  // for Sharp 2Y0A02
  return 60.374 * pow(map(value, 0, 1023, 0, 5000)/1000.0, -1.16);
}

// Variables
long nextReportTime = 0;
Averager<averagingSamples> averager;
long nextAnimationTime = 0;
int latestDistance = 666;
State state;

void setup() {
  pinMode(Pins::LedR, OUTPUT);
  pinMode(Pins::LedG, OUTPUT);
  pinMode(Pins::LedB, OUTPUT);
  pinMode(Pins::DistanceSensor, INPUT);
  Serial.begin(9600);
  const auto &c = input.idleColor;
  analogWrite(Pins::LedR, c.r);
  analogWrite(Pins::LedG, c.g);
  analogWrite(Pins::LedB, c.b);
}

void loop() {
  // Read the sensor values
  const int val = analogRead(Pins::DistanceSensor);
  averager.push(val);

  // Report sensor value, and update latestDistance
  const long currentTime = millis();
  if (currentTime >= nextReportTime) {
    static const int BUF_MAX = 50;
    char buf[BUF_MAX] = {0};
    latestDistance = calculateDistanceCm(averager.getMedian());
    snprintf(buf, BUF_MAX, "updated distance=%d\n", latestDistance);
    Serial.write(buf);
    nextReportTime = currentTime+reportIntervalMs;
  }

  // Update animation
  const long timePreAnimation = millis();
  if (timePreAnimation > nextAnimationTime) {
    // Update state
    input.idle = latestDistance > idleDistanceCm;

    // Forward time
    input.timeMs += animationIntervalMs;

    // WARN: does not respect parser.color (or visa verca)
    const State next = nextState(input, state);
    const RgbColor c = next.ledColor;
    analogWrite(Pins::LedR, c.r);
    analogWrite(Pins::LedG, c.g);
    analogWrite(Pins::LedB, c.b);

    nextAnimationTime = timePreAnimation+animationIntervalMs;
    state = next;
  }
}
