
struct Pins {
  const static int LedR = 6;
  const static int LedG = 3;
  const static int LedB = 5;
};

void setup() {
  pinMode(Pins::LedR, OUTPUT);
  pinMode(Pins::LedG, OUTPUT);
  pinMode(Pins::LedB, OUTPUT);
  Serial.begin(9600);
  while (!Serial) {
    ; // wait for serial port to connect. Needed for native USB
  }
  Serial.write("ready\n");
}

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
      index = 0;
    }
    buffer[index++] = data;

    if (data == '\n') {
      int r = -1;
      int g = -1;
      int b = -1;
      const int read = sscanf(buffer, "set r=%d, g=%d, b=%d\n", &r, &g, &b, BUFFER_MAX);
      Serial.write("read records: ");
      Serial.write(read);
      Serial.write("\n");
      if (read == 3) {
        color.r = constrain(r, 0, 255);
        color.g = constrain(g, 0, 255);
        color.b = constrain(g, 0, 255);
        return true;
      } else {
        return false;
      }
    } else {
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


void loop() {
  Parser parser;
  parser.color = { 0, 0, 0 };
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
      snprintf(buf, BUF_MAX, "updated r=%d, g=%d, b=%d\n", c.r, c.g, c.b);
      Serial.write(buf);
    }
  }
}
