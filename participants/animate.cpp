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
    void
    updateInput() {
        // FIXME: implement
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
    def.outports = {
        { "out", "array", "" },
        { "ledcolor", "array", "" },
    };
    def.role = role;

    msgflo::Participant *participant = engine->registerParticipant(def, [&](msgflo::Message *msg) {
        auto payload = msg->asJson();
        std::cout << "Got message:" << endl << payload.dump() << std::endl;
        participant->send("out", payload);
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
    Input initial = {
        timeMs: 1,
        breathingPeriodMs: 2100,
        breathingColor: { 200, 200, 255 },
        heartRate: 80,
        heartbeatColor: { 255, 10, 10 },
        heartbeatLengthMs: 100,
    };

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
