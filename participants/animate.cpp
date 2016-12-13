#include <atomic>
#include <iostream>
#include <algorithm>
#include <chrono>
#include <thread>

#include <msgflo.h>
#include "animation.hpp"

using namespace std;

class Animator {
public:
    Animator(Input in, InteractionInput iin)
      : input(in)
      , interact(iin)
    {
        stateChanged = [](State s) {
            cerr << "Animator:: No state change handler attached";
        };

        setDistance(interact.distanceCm);
    }

    void
    run(int forwardTime) {
        input.timeMs += forwardTime;

        const auto s = interactionState.current;
        input.breathingPeriodMs = s.breathingPeriod;
        input.heartRate = s.heartRate;
        input.idle = s.aboveThreshold;

        const State next = nextState(input, state);
        realizeState(state, config);
        state = next;
    }

    void setDistance(int distanceCm) {
        interact.distanceCm = distanceCm;

        auto n = interactionNext(interact, interactionState);
        interactionState = n;
    }

    Input
    getInput() {
       return input;
    }
    void
    setInput(const Input & in) {
       input = in;
    }

    bool
    realizeState(const State& state, const Config &config) {
        stateChanged(state);
        return true;
    }

    void
    onStateChanged(std::function<void(State)> callback) {
        stateChanged = callback;
    }

public:
    Input input;
    InteractionInput interact;
    InteractionState interactionState;
private:
    State state;
    Config config;
    std::function<void(State)> stateChanged;
};

bool
validInput(int val) {
    return (val > 3 && val < 10000); 
}

void
setupAnimator(Animator *animator, const std::string &role, std::shared_ptr<msgflo::Engine> engine) {

    msgflo::Definition def;
    def.component = "AnimateRebirth";
    def.label = "Repeats input on outport unchanged";
    def.inports = {
        //{ "startstop", "boolean", "" },

        // Primary input
        { "distance", "number", "" },

        // Tuning
        { "threshold", "number", "" },
        { "idlecolor", "color", "" },
        { "breathingcolor", "color", "" },
        { "heartbeatcolor", "color", "" },

        { "heartbeatlength", "number", "" },
        { "activeheartrate", "number", "" },
        { "activebreathingperiod", "number", "" },
    };
    def.outports = {

        // derived state
        { "abovethreshold", "boolean", "" },
        { "breathingperiod", "number", "" },
        { "heartrate", "number", "" },

        { "error", "error", "" },
        { "distancechanged", "number", "" },
        { "configchanged", "object", "" },
        { "ledcolor", "array", "" },
    };
    def.role = role;

    msgflo::Participant *participant = engine->registerParticipant(def);

    auto updateInteractionInputs = [participant, animator](int distanceCm) {
        animator->setDistance(distanceCm);

        auto current = animator->interactionState.current; 
        participant->send("heartrate", std::to_string(current.heartRate));
        participant->send("breathingperiod",
            std::to_string(current.breathingPeriod));
        participant->send("abovethreshold",
            current.aboveThreshold ? std::string("true") : std::string("false"));

        participant->send("distancechanged", std::to_string(distanceCm));
    };

    participant->onMessage([participant, animator, updateInteractionInputs](msgflo::Message *msg) {
        std::string port = msg->port();

        auto payload = msg->asString();
        auto c = animator-> getInput();

        if (port == "distance") {
            // Calculate new parameters for animation
            const int distanceCm = stoi(payload);
            if (distanceCm > 0 && distanceCm < 8000) {
                updateInteractionInputs(distanceCm);
            } else {
                auto error = std::string("distance outside valid range: ") + std::to_string(distanceCm);
                participant->send("error", error);
            }

        } else if (port == "threshold") {
            const int value = stoi(payload);
            if (value > 0 && value < 1000) {
                animator->interact.distanceThresholdCm = value;
                updateInteractionInputs(animator->interact.distanceCm);   
            } else {
                auto error = std::string("threshold outside valid range: ") + std::to_string(value);
                participant->send("error", error);
            }

        // settings
        } else if (port == "interpolationperiod") {
            const int interpolate = stoi(payload);
            animator->interact.interpolationPeriodMs = interpolate;
            participant->send("configchanged", std::string("TODO"));

        } else if (port == "heartbeatcolor") {
            c.heartbeatColor = RgbColor::fromHexString(payload.c_str());
            animator->setInput(c);
            participant->send("configchanged", c);
        } else if (port == "breathingcolor") {
            c.breathingColor = RgbColor::fromHexString(payload.c_str());
            animator->setInput(c);
            participant->send("configchanged", c);
        } else if (port == "idlecolor") {
            c.idleColor = RgbColor::fromHexString(payload.c_str());
            animator->setInput(c);
            participant->send("configchanged", c);
        } else if (port == "heartbeatlength") {
            c.heartbeatLengthMs = stoi(payload.c_str());
            animator->setInput(c);
            participant->send("configchanged", c);
        } else if (port == "activeheartrate") {
            const int value = stoi(payload.c_str());
            animator->interact.activeHeartRate = value;
            participant->send("configchanged", c);
        } else if (port == "activebreathingperiod") {
            const int value = stoi(payload.c_str());
            animator->interact.activeBreathingPeriod = value;
            participant->send("configchanged", c);
        } else {
            std::string error = "Changing " + port + " not implemented";
            participant->send("error", error);
        }

        msg->ack();
    });

    animator->onStateChanged([participant](State state) {
        participant->send("ledcolor", state.ledColor.to_json());
    });
    return;
}

int main(int argc, char **argv)
{
    // Parse options
    std::string role = "animate";
    if (argc >= 2) {
        role = argv[1];
    }

    msgflo::EngineConfig config;
    if (argc >= 3) {
        config.url(argv[2]);
    }

    int tickMs = 100;
    const char *tick = std::getenv("REBIRTH_TICKMS");
    if (tick) {
        tickMs = std::stoi(tick);
    }

    // Setup
    Input initial = initialInputConfig();
    auto intera = initialInteractionConfig();    
    Animator animator(initial, intera);
    auto engine = msgflo::createEngine(config);
    setupAnimator(&animator, role, engine);

    // Run
    atomic_bool run(true);
    std::thread updater([&]()
    {
        while (run) {
            this_thread::sleep_for(chrono::milliseconds(tickMs));
            animator.run(tickMs);
        }
    });

    std::cout << role << "(AnimateRebirth) started" << endl;
    engine->launch();

    return EXIT_SUCCESS;
}
