

struct Pins {
  const static int LedR = 9;
  const static int LedG = 10;
  const static int LedB = 11;
  const static int DistanceSensor = 0; // analog in
};

struct RgbColor {
  uint8_t r;
  uint8_t g;
  uint8_t b;
#ifdef HAVE_JSON11
  json11::Json to_json() const {
    return json11::Json::object {
        {"r", r}, {"g", g}, {"b", b},
    };
  }
#endif
};

class Parser {
public:
  bool push(uint8_t data) {
    if (index >= BUFFER_MAX) {
      // prevent overflowing buffer
      index = 0;
    }
    buffer[index++] = data;

    if (data == '\n') {
      // parse the command
      int r = -1;
      int g = -1;
      int b = -1;
      const int read = sscanf(buffer, "set r=%d g=%d b=%d\n", &r, &g, &b, BUFFER_MAX);
      index = 0;
      memset(buffer, BUFFER_MAX, 0);
      if (read == 3) {
        // All values were present
        color.r = constrain(r, 0, 255);
        color.g = constrain(g, 0, 255);
        color.b = constrain(b, 0, 255);
        return true;
      } else {
        // ERROR: Incorrect format
        return false;
      }
    } else {
      // wait, not end of command yet
      return false;
    }
    
  }

private:
  static const size_t BUFFER_MAX = 100;
  char buffer[BUFFER_MAX] = {};
  int index = 0;
public:
  RgbColor color;
};


int int_cmp(const void *a, const void *b) { 
  const int *ia = (const int *)a;
  const int *ib = (const int *)b;
  return *ia  - *ib;
}

template <int Length>
class Averager {
public:
  // collect values
  void push(int value) {
    buffer[index++] = value;
    if (index >= size) {
      index = 0;
    }
  }
  // calculate median of the values
  // NOTE: if not completely filled yet, will give arbitrary result
  int getMedian() {
    int samples[size];
    qsort(samples, size, sizeof(int), int_cmp);
    return samples[size/2]; 
  }

private:
  int buffer[Length];
  const int size = Length;
  int index = 0;
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
