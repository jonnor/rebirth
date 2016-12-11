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
