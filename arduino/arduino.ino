
struct Pins {
  const static int LedR = 9;
  const static int LedG = 10;
  const static int LedB = 11;
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

// Arduino code
Parser parser;

void setup() {
  pinMode(Pins::LedR, OUTPUT);
  pinMode(Pins::LedG, OUTPUT);
  pinMode(Pins::LedB, OUTPUT);
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
  
  if (Serial.available() > 0) {
    const uint8_t in = Serial.read();
    const bool changed = parser.push(in);
    if (changed) {
      auto &c = parser.color;
      analogWrite(Pins::LedR, c.r);
      analogWrite(Pins::LedG, c.g);
      analogWrite(Pins::LedB, c.b);
      static const int BUF_MAX = 100;
      char buf[BUF_MAX] = {0};
      snprintf(buf, BUF_MAX, "updated r=%d g=%d b=%d\n", c.r, c.g, c.b);
      Serial.write(buf);
    }
  }
}
