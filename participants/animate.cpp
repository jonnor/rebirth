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
    Animator(Input in)
      : input(in)
    {
        stateChanged = [](State s) { cerr << "Animator:: No state change handler attached"; };
    }

    void
    run(int forwardTime) {
        input.timeMs += forwardTime;
        const State next = nextState(input, state);
        realizeState(state, config);
        state = next;
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

private:
    Input input;
    State state;
    Config config;
    std::function<void(State)> stateChanged;
};

void
setupAnimator(Animator *animator, const std::string &role, std::shared_ptr<msgflo::Engine> engine) {

    msgflo::Definition def;
    def.component = "AnimateRebirth";
    def.label = "Repeats input on outport unchanged";
    def.inports = {
        //{ "startstop", "boolean", "" },

        // Primary inputs
        { "heartrate", "number", "" },
        { "breathingperiod", "number", "" },

        // Tuning
        { "breathingcolor", "color", "" },
        { "heartbeatcolor", "color", "" },
        { "heartbeatlength", "number", "" },
    };
    def.outports = {
        { "error", "error", "" },
        { "configchanged", "object", "" },
        { "ledcolor", "array", "" },
    };
    def.role = role;

    msgflo::Participant *participant = engine->registerParticipant(def);
    participant->onMessage([participant, animator](msgflo::Message *msg) {
        std::string port = msg->port();

        auto c = animator-> getInput();
        if (port == "heartrate") {
            auto payload = msg->asString();
            c.heartRate = stoi(payload);
            animator->setInput(c);
            participant->send("configchanged", c);
        } else if (port == "breathingperiod") {
            auto payload = msg->asString();
            c.breathingPeriodMs = stoi(payload);
            animator->setInput(c);
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

    // Setup
    Input initial = initialInputConfig();

    Animator animator(initial);

    auto engine = msgflo::createEngine(config);
    setupAnimator(&animator, role, engine);

    int tickMs = 100;
    const char *tick = std::getenv("REBIRTH_TICKMS");
    if (tick) {
        tickMs = std::stoi(tick);
    }

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
