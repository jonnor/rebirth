// Experiment in using FRP-like techniques for embedded systems,
// applied to some LED animations

#include <stdint.h>
#include <assert.h>
#include <stdio.h>

#ifdef DO_SIMULATION
#include <stdio.h>
#include <vector>
#include <string>
#endif

#ifdef HAVE_JSON11
#include <json11.hpp>
#include <json11.cpp>
#include "./flowtrace.hpp"
#endif

#include "./color.hpp"


static inline int
mix(uint8_t A, uint8_t B, int a, int b, int total) {
    return (A*a + B*b) / (total);
}

//
RgbColor
mix(RgbColor a, RgbColor b, int balance) {
    const int total = 1000;
    const int bMix = balance;
    const int aMix = total-balance;
    RgbColor ret = {
        mix(a.r, b.r, aMix, bMix, total),
        mix(a.g, b.g, aMix, bMix, total),
        mix(a.b, b.b, aMix, bMix, total),
    };
    return ret;
}

// Anything that may influence the system (cause state to change)
struct Input {
    long timeMs;

    int breathingPeriodMs;
    RgbColor breathingColor;

    int heartRate; // BPM
    RgbColor heartbeatColor;
    int heartbeatLengthMs;

#ifdef HAVE_JSON11
    json11::Json to_json() const {
        using namespace json11;
        return Json::object {
            {"timeMs", (int)timeMs},
            {"breathingPeriodMs", breathingPeriodMs},
            {"breathingColor", breathingColor.to_json()},
    };
}
#endif
};

// State of system
struct State {
    RgbColor ledColor;
    
    long timeMod;

#ifdef HAVE_JSON11
    json11::Json to_json() const {
        using namespace json11;
        return Json::object {
            {"timeMod", (double)timeMod},
        };
    }
#endif

};

struct Config {
    bool _;
};



#define between(val, lower, upper) (val >= lower && val <= upper) ? 1 : 0

long map(long x, long in_min, long in_max, long out_min, long out_max)
{
  return (x - in_min) * (out_max - out_min) / (in_max - in_min) + out_min;
}

#define constrain(amt,low,high) ((amt)<(low)?(low):((amt)>(high)?(high):(amt)))


static inline int
scale(uint8_t i, int factor, int max) {
    return (i*factor)/max;    
}

RgbColor
scaleBrightness(RgbColor c, int factor, int max) {
    return {
        scale(c.r, factor, max),
        scale(c.r, factor, max),
        scale(c.r, factor, max),
    };
}

//|     /| /     |
//|    / |/      |
int
sawWave(long time, int period, int low, int high) {
    const long pos = time % period;
    return constrain(map(pos, 0, period, low, high), low, high);
}

//|    /\  /\     |
//|   /  \/  \    |
// Note: a bit unorthodox in that we go from bottom to bottom
int
triangleWave(long time, int period, int low, int high) {
    const int halfPeriod = (period/2);
    const long pos = time % period; 

    if (pos < halfPeriod) {
        // moving upwards
        const long p = time % halfPeriod;
        return map(p, 0, halfPeriod, low, high);
    } else {
        // moving downwards
        const long p = (time+halfPeriod) % halfPeriod;
        return map(p, 0, halfPeriod, high, low);
   }
}

State
nextState(const Input &input, const State& previous) {
    State s = previous;

    // Breathing
    const int max = 255;
    const long mod = triangleWave(input.timeMs, input.breathingPeriodMs, 0, max);
    RgbColor breathing = scaleBrightness(input.breathingColor, mod, max);
    s.timeMod = mod;

    // Heartbeat
    RgbColor heartbeat = input.heartbeatColor;
    const long heartbeatPeriod = (1000*60)/input.heartRate;
    const long heartbeatPos = input.timeMs % heartbeatPeriod;
    const int heartbeatMix = ( between(heartbeatPos, 1, input.heartbeatLengthMs) ) ?  1000 : 0;

    // Combine
    s.ledColor = mix(breathing, heartbeat, heartbeatMix);

    return s;
}

#ifdef DO_SIMULATION
// Render using console Truecolor https://gist.github.com/XVilka/8346728
void
colorRenderTerminal(RgbColor color, const std::string &text) {
    const int background = 2;
    printf("\x1b[38;%d;%d;%d;%dm%s\x1b[0m",
           background, color.r, color.g, color.b, text.c_str());

}

bool
realizeState(const State& state, const Config &config) {
    // TODO: implement for hardware
    // TODO: implement via MsgFlo, sending MQTT message to HW-unit
    colorRenderTerminal(state.ledColor, "(########)");
    printf("%s\n", state.to_json().dump().c_str());
    return true;
}

// TODO: serialize inputs and outputs as a Flowtrace

bool
tests(void) {
    RgbColor midGray = { 100, 100, 100 };
    RgbColor pureRed = { 255, 0, 0 }; 
    
    // mix()
    RgbColor fullA = mix(midGray, pureRed, 0);
    assert(fullA == midGray);

    RgbColor fullB = mix(midGray, pureRed, 1000);
    assert(fullB == pureRed);

    RgbColor mid = mix(midGray, pureRed, 500);
    RgbColor midExpected{ 177, 50, 50 };
    //fprintf(stderr, "%s\n", mid.to_string().c_str());
    assert(mid == midExpected);

    RgbColor quarterA = mix(midGray, pureRed, 250);
    //fprintf(stderr, "%s\n", quarterA.to_string().c_str());
    assert(quarterA == (RgbColor{ 138, 75, 75 }));

    // scaleBrightness()
    RgbColor noBright = scaleBrightness(midGray, 0, 255);
    //fprintf(stderr, "%s\n", quarterA.to_string().c_str());
    assert(noBright == (RgbColor{ 0, 0, 0 }));

    RgbColor halfBright = scaleBrightness(midGray, 128, 255);
    //fprintf(stderr, "%s\n", halfBright.to_string().c_str());
    assert(halfBright == (RgbColor{ 50, 50, 50 }));

    RgbColor fullBright = scaleBrightness(midGray, 255, 255);
    //fprintf(stderr, "%s\n", fullBright.to_string().c_str());
    assert(fullBright == midGray);

    // triangleWave()
    const int waveMin = 10;
    const int waveMax = 255;
    const int oneSecondMs = 1000; 
    auto triangleZero = triangleWave(0, oneSecondMs, waveMin, waveMax);
    assert(triangleZero == waveMin);

    auto triangleOnePeriod = triangleWave(oneSecondMs,
                                oneSecondMs, waveMin, waveMax);
    //fprintf(stderr, "%d\n", triangleOnePeriod);
    assert(triangleOnePeriod == waveMin);

    auto triangleHalfUppeak = triangleWave((oneSecondMs/2)-10,
                                oneSecondMs, waveMin, waveMax);
    //fprintf(stderr, "%d\n", triangleHalfUppeak);
    assert(triangleHalfUppeak == waveMax-5);

    auto triangleHalf = triangleWave((oneSecondMs/2),
                            oneSecondMs, waveMin, waveMax);
    //fprintf(stderr, "%d\n", triangleHalf);
    assert(triangleHalf == waveMax);

    auto triangleHalfDownpeak = triangleWave((oneSecondMs/2)+10,
                                oneSecondMs, waveMin, waveMax);
    //fprintf(stderr, "%d\n", triangleHalfDownpeak);
    assert(triangleHalfDownpeak == waveMax-4);

    return true;
}

struct InputOutputPair {
    Input input;    
    State state;
};

json11::Json
createFlowtrace(std::vector<InputOutputPair> history) {

    std::vector<flowtrace::Event> events;

    for (auto &p : history) {
        // TODO: take time into account here
        flowtrace::Event in(p.input);
        flowtrace::Event out(p.state);
        events.push_back(in);
        events.push_back(out);
    }

    // TODO: put a graph representation into the header
    using namespace json11;   
    auto graph = Json::object {};

    return Json::object {
        { "header", Json::object {
            { "graphs", Json::object {
                { "default", graph },
            }},
        }},
        { "events", events },
    };
}

int
main(int argc, char *argv[]) {

    std::vector<InputOutputPair> history;

    const int simulationInterval = 100;
    const int simulationTime = 10*1000;

    tests();

    int currentTime = 0;
    State previousState;
    Input in = {
        timeMs: currentTime,
        breathingPeriodMs: 2100,
        breathingColor: { 200, 200, 255 },
        heartRate: 80,
        heartbeatColor: { 255, 10, 10 },
        heartbeatLengthMs: 100,
    };
#ifdef HAVE_JSON11
    printf("%s", in.to_json().dump().c_str());
#endif
    Config config;
    while (currentTime < simulationTime) {
        in.timeMs = currentTime;
        const State state = nextState(in, previousState);
        realizeState(state, config);
        history.push_back({ in, state });
        previousState = state;
        currentTime += simulationInterval;
    }

    auto trace = createFlowtrace(history);
    // TODO: write to file
    //printf("%s", trace.dump().c_str());
}
#endif
