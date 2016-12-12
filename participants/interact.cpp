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
    Animator(InteractionInput in)
      : input(in)
    {
        stateChanged = [](InteractionState s) { cerr << "Animator:: No state change handler attached"; };
    }

    void
    run(int forwardTime) {
        input.timeMs += forwardTime;
        const auto next = interactionNext(input, state);
        stateChanged(state);
        state = next;
    }

    InteractionInput
    getInput() {
       return input;
    }
    void
    setInput(const InteractionInput & in) {
       input = in;
    }

    void
    onStateChanged(std::function<void(InteractionState)> callback) {
        stateChanged = callback;
    }

private:
    InteractionInput input;
    InteractionState state;
    std::function<void(InteractionState)> stateChanged;
};

void
setupAnimator(Animator *animator, const std::string &role, std::shared_ptr<msgflo::Engine> engine) {

    msgflo::Definition def;
    def.component = "Interact";
    def.label = "Calculates ";
    def.inports = {
        { "distance", "number", "" },
    };
    def.outports = {
        { "error", "error", "" },
        { "configchanged", "object", "" },
        { "distancechanged", "number", "" },
        { "heartrate", "number", "" },
        { "breathingperiod", "number", "" },
    };
    def.role = role;

    msgflo::Participant *participant = engine->registerParticipant(def);
    participant->onMessage([participant, animator](msgflo::Message *msg) {
        std::string port = msg->port();

        auto payload = msg->asString();
        auto c = animator-> getInput();
        if (port == "distance") {
            c.distanceCm = stoi(payload);
            animator->setInput(c);
            participant->send("distancechanged", std::to_string(c.distanceCm));

        } else if (port == "interpolationperiod") {
            c.interpolationPeriodMs = stoi(payload);
            animator->setInput(c);
            participant->send("configchanged", std::string("TODO"));

        } else {
            std::string error = "Changing " + port + " not implemented";
            participant->send("error", error);
        }

        msg->ack();
    });

    animator->onStateChanged([participant](InteractionState state) {
        participant->send("heartrate",
            std::to_string(state.current.heartRate));
        participant->send("breathingperiod",
            std::to_string(state.current.breathingPeriod));
    });
    return;
}

int main(int argc, char **argv)
{
    // Parse options
    std::string role = "interact";
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
    InteractionInput initial = {
        timeMs: 1,
        distanceCm: 300,
        interpolationPeriodMs: 1000, 
    };
    Animator animator(initial);
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

    std::cout << role << "(Interact) started" << endl;
    engine->launch();

    return EXIT_SUCCESS;
}
