// Experiment in using FRP-like techniques for embedded systems,
// applied to some LED animations

#ifndef REBIRTH_ANIMATION_HPP
#define REBIRTH_ANIMATION_HPP

#include <stdint.h>

#ifdef HAVE_JSON11
#include <json11.hpp>
#include <json11.cpp>
#endif

#include "color.hpp"


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
            //{"timeMs", (int)timeMs},
            {"breathingPeriodMs", breathingPeriodMs},
            {"heartRate", heartRate},
            //{"breathingColor", breathingColor.to_json()},
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

Input initialInputConfig() {
    static const Input initial = {
        timeMs: 1,
        breathingPeriodMs: 2100,
        breathingColor: { 200, 200, 255 },
        heartRate: 80,
        heartbeatColor: { 255, 10, 10 },
        heartbeatLengthMs: 100,
    };
    return initial;
}


struct InteractionOutput {
    int heartRate;
    int breathingPeriod;
};
struct InteractionInput {
    long timeMs;
    int distanceCm;
    int interpolationPeriodMs;
};
struct InteractionState {
    long targetTimeMs;
    InteractionOutput target;
    InteractionOutput current;
};

int
interpolate(long distance, int from, int to, int maxDistance) {
    if (distance < 0) {
        return to;
    }
    const long fromTarget = maxDistance-distance;
    return map(fromTarget, 0, maxDistance, from, to);
}

bool
basicallySame(InteractionOutput a, InteractionOutput b) {
    const bool sameRate = abs(a.heartRate - b.heartRate) < 10;
    const bool samePeriod = abs(a.breathingPeriod - b.breathingPeriod) < 100;
    return sameRate && samePeriod;
}

InteractionState
interactionNext(const InteractionInput &in, const InteractionState& prev) {
    // calculate targets from input
    // TODO: scale intensity up by inverse proportion of distance
    const int maxDistance = 120;
    InteractionOutput desired;
    if (in.distanceCm > maxDistance) {
        desired.heartRate = 50;
        desired.breathingPeriod = 2100;
    } else {
        desired.heartRate = 133;
        desired.breathingPeriod = 900;
    }

    InteractionState state = prev;
    if (basicallySame(desired, prev.current)) {
        // difference is small, can jump straight to new
        state.current = desired;
    } else {
        if (!basicallySame(desired, prev.target)) {
            // target changed and we need (more) time to get there
            state.targetTimeMs = in.timeMs + in.interpolationPeriodMs;
        }
        state.target = desired;

        // interpolation
        InteractionOutput i;
        InteractionOutput from = prev.current;
        const long dist = state.targetTimeMs - in.timeMs;
        const long period = in.interpolationPeriodMs;
        i.heartRate = interpolate(dist, from.heartRate, desired.heartRate, period);
        i.breathingPeriod = interpolate(dist, from.breathingPeriod, desired.breathingPeriod, period);

        // TODO: test and actually use interpolation
        state.current = i;
        state.current = desired;
    }

    return state;
}


#endif //REBIRTH_ANIMATION_HPP
