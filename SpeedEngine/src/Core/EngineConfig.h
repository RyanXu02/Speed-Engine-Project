#pragma once

#include <string>

namespace SE 
{
	struct EngineConfig
	{
		std::string windowTitle = "Speed Engine";

		int windowWidth = 1280;
		int windowHeight = 720;
		bool fullscreen = false;

		std::string assetDirectory = "./assets";
	};
}