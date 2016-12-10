#include <atomic>
#include <iostream>
#include <algorithm>
#include <chrono>
#include <thread>

#include "msgflo.h"

#include "color.hpp"

using namespace std;

// Render using console Truecolor https://gist.github.com/XVilka/8346728
void
colorRenderTerminal(RgbColor color, const std::string &text) {
    const int background = 2;
    printf("\x1b[38;%d;%d;%d;%dm%s\x1b[0m",
           background, color.r, color.g, color.b, text.c_str());

}

int main(int argc, char **argv)
{
    std::string role = "show";
    if (argc >= 2) {
        role = argv[1];
    }

    msgflo::Definition def;
    def.component = "ShowColor";
    def.label = "Shows an RGB color in the terminal";
    def.outports = {
        { "out", "object", "" },
        { "error", "error", "" },
    };
    def.role = role;

    msgflo::EngineConfig config;
    if (argc >= 3) {
        config.url(argv[2]);
    }

    auto engine = msgflo::createEngine(config);
    msgflo::Participant *participant = engine->registerParticipant(def, [&](msgflo::Message *msg) {
        auto payload = msg->asJson();
        
        if (payload["r"].is_number() && payload["g"].is_number() && payload["b"].is_number()) {
            RgbColor color = {
                payload["r"].number_value(),
                payload["g"].number_value(),
                payload["b"].number_value(),
            };
            colorRenderTerminal(color, "XXXXXXXXXX\n");
            fflush(stdout);
        } else {
            auto error = "input was not a color: " + payload.dump();
            participant->send("error", error);
        }        
        
        msg->ack();
    });

    std::cout << def.role << " started" << endl;
    engine->launch();

    return EXIT_SUCCESS;
}
