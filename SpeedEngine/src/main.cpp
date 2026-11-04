#include "pch.h"

#include "Core/Engine.h"

#if defined(_WIN32)
extern "C" {
	__declspec(dllexport) unsigned long NvOptimusEnablement = 0x00000001;
	__declspec(dllexport) unsigned long AmdPowerXpressRequestHighPerformance = 0x00000001;
}
#endif


int main()
{

	SE::Engine& engine = SE::Engine::Instance();
	engine.init();
	engine.start();
	engine.run();
	engine.stop();

	return 0;
}
