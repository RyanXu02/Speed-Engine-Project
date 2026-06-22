#include "pch.h"

#include "Core/Logger/Logger.h"

#include "glm.hpp"
#include "glad/gl.h"
#include "spdlog.h"
#include "imgui.h"
#include "fmod.hpp"

// Jolt includes
#include <Jolt/RegisterTypes.h>
#include <Jolt/Core/Factory.h>
#include <Jolt/Core/TempAllocator.h>
#include <Jolt/Core/JobSystemThreadPool.h>
#include <Jolt/Physics/PhysicsSettings.h>
#include <Jolt/Physics/PhysicsSystem.h>
#include <Jolt/Physics/Collision/Shape/BoxShape.h>
#include <Jolt/Physics/Collision/Shape/SphereShape.h>
#include <Jolt/Physics/Body/BodyCreationSettings.h>
#include <Jolt/Physics/Body/BodyActivationListener.h>



int main() {
    
    std::queue<int> myQueue;
	myQueue.push(1);
	printf("Hello world! %d\n", myQueue.front());

	glm::vec3 test = glm::vec3(1.0f, 2.0f, 3.0f);
	printf("Hello world! %f\n", test.x);

    JPH::RegisterDefaultAllocator();

    SE::Logger logger;
    logger.init("CoolLLogger");
    logger.critical("test");

    //SN::AppConfig appConfig{};
    //appConfig.title = "Slippery Slope";
    //appConfig.windowPlatform = SN::WindowPlatform::GLFW;
    //appConfig.renderPlatform = SN::RenderPlatform::OPENGL;
    //appConfig.guiPlatform = SN::GuiPlatform::IMGUI;
    //appConfig.os = SN::OperatingSystem::WINDOW;
    //appConfig.audioPlatform = SN::AudioPlatform::FMOD;
    //appConfig.width = 1920;
    //appConfig.height = 1080;
    //appConfig.vsync = false;    // set to true to fix screen tearing but currently breaks physics
    //appConfig.developmentMode = false;
    //appConfig.updateFPS = 120.0;
    //appConfig.renderFPS = 120.0;


    //SN::Engine engine(appConfig);
    //engine.init();
    //engine.start();
    //engine.run();
    //engine.shutDown();

    return 0;
}
