#include <atomic>
#include <iostream>
#include <algorithm>
#include <chrono>
#include <thread>

#include <msgflo.h>
#include "../arduino/animation.hpp"

using namespace std;

/*
    const int simulationInterval = 200;
    const int simulationTime = 5*1000;

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
        history.push_back(in);
        previousState = state;
        currentTime += simulationInterval;
    }
*/

struct ParticipantState {
    Input in;
    State state;
};

void
setupAnimate(ParticipantState *state, const std::string &role, std::shared_ptr<msgflo::Engine> engine) {

    msgflo::Definition def;
    def.component = "AnimateRebirth";
    def.label = "Repeats input on outport unchanged";
    def.outports = {
        { "out", "array", "" }
    };
    def.role = role;
    msgflo::Participant *participant = engine->registerParticipant(def, [&](msgflo::Message *msg) {
        auto payload = msg->asJson();
        std::cout << "Got message:" << endl << payload.dump() << std::endl;
        participant->send("out", payload);
        msg->ack();
    });
    return;
}

atomic_bool run(true);
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

    // Run  
    auto engine = msgflo::createEngine(config);
    ParticipantState state;
    setupAnimate(&state, role, engine);

    std::cout << role << "(AnimateRebirth) started" << endl;
    engine->launch();

    return EXIT_SUCCESS;
}
