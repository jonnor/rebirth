
#include <stdio.h>
#include <vector>
#include <string>

#include "animation.hpp"

#include "./flowtrace.hpp"

// Render using console Truecolor https://gist.github.com/XVilka/8346728
void
colorRenderTerminal(RgbColor color, const std::string &text) {
    const int background = 2;
    printf("\x1b[38;%d;%d;%d;%dm%s\x1b[0m",
           background, color.r, color.g, color.b, text.c_str());

}

bool
realizeState(const State& state, const Config &config) {
    colorRenderTerminal(state.ledColor, "(########)");
    printf("%s\n", state.to_json().dump().c_str());
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

std::vector<InputOutputPair>
simulateAnimation() {
    std::vector<InputOutputPair> history;

    const int simulationInterval = 100;
    const int simulationTime = 10*1000;

    int currentTime = 0;
    State previousState;
    Input in = initialInputConfig();
#ifdef HAVE_JSON11
    printf("%s", in.to_json().dump().c_str());
#endif
    Config config;
    while (currentTime < simulationTime) {
        in.timeMs = currentTime;
        in.idle = false;
        const State state = nextState(in, previousState);
        realizeState(state, config);
        history.push_back({ in, state });
        previousState = state;
        currentTime += simulationInterval;
    }

    return history;
}
