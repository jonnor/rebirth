
struct RgbColor {
    uint8_t r;
    uint8_t g;
    uint8_t b;
#ifdef HAVE_JSON11
    json11::Json to_json() const {
        return json11::Json::object {
            {"r", r},
            {"g", g},
            {"b", b},
    };
}
    std::string to_string() {
        char buf[100];
        snprintf(buf, 100, "RgbColor(%d, %d, %d)", r, g, b);
        return std::string(buf);
    }
#endif

    bool operator==(const RgbColor &other) {
        const bool equals = \
            (r == other.r) &&
            (g == other.g) &&
            (b == other.b); 
        return equals;
    }
};
