#include "pch.h"
#include "Core/Engine.h"
#include "Core/Logger/Logger.h"

int main() {

    SE::Engine engine;
    engine.init();
    engine.start();
    // engine.run();
    engine.stop();

    return 0;
}
