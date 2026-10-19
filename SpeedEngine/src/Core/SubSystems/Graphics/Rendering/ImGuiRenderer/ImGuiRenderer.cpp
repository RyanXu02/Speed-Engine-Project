#include "pch.h"

#if defined(COMPILE_WITH_EDITOR)
#include "Core/SubSystems/ImGui/Widget.h"
#include "Core/SubSystems/ImGui/Widgets/ConsoleWidget.h"
#include "Core/SubSystems/ImGui/Widgets/MainWidget.h"
#include "Core/SubSystems/ImGui/Widgets/MemoryWidget.h"
#include "Core/SubSystems/ImGui/Widgets/PropertiesWidget.h"
#include "Core/SubSystems/ImGui/Widgets/ResourceBrowserWidget.h"
#include "Core/SubSystems/ImGui/Widgets/SceneInfoWidget.h"
#include "Core/SubSystems/ImGui/Widgets/ScenePreviewWidget.h"
#include "Core/SubSystems/Graphics/Window.h"
#include "ImGuiRenderer.h"
#include "imgui.h"

#include <imgui_impl_glfw.h>
#include <imgui_impl_opengl3.h>

namespace SE
{
	void ImGuiRenderer::init()
	{
		m_logger.info("Initializing");

		IMGUI_CHECKVERSION();
		ImGui::CreateContext();
		ImGuiIO& io = ImGui::GetIO();

		std::error_code ec;
		const bool hasUserIni = std::filesystem::exists("imgui.ini", ec);
		if (!ec && !hasUserIni)
		{
			ImGui::LoadIniSettingsFromDisk("default_UI.ini");
		}

		io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard; // Enable Keyboard Controls
		io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad; // Enable Gamepad Controls
		io.ConfigFlags |= ImGuiConfigFlags_DockingEnable; // IF using Docking Branch

		// Setup Platform/Renderer backends
		ImGui_ImplGlfw_InitForOpenGL(m_window->getWindow(),
		    true); // Second param install_callback=true will install GLFW callbacks
		// and chain to existing ones.
		ImGui_ImplOpenGL3_Init();

		initWidgets();
	}

	void ImGuiRenderer::initWidgets()
	{
		createWidget<MainWidget>();
		createWidget<ResourceBrowserWidget>();
		createWidget<MemoryWidget>();
		// createWidget<ConsoleWidget>();
		createWidget<ScenePreviewWidget>();
		createWidget<SceneInfoWidget>();
		createWidget<PropertiesWidget>();
		// ...
	}

	void ImGuiRenderer::update(double deltaTime)
	{
		ImGui_ImplOpenGL3_NewFrame();
		ImGui_ImplGlfw_NewFrame();
		ImGui::NewFrame();
		ImGui::ShowDemoWindow(); // Show demo window! :)
	}

	void ImGuiRenderer::render() const
	{
		for (const auto& [id, widget] : m_widgets)
		{
			if (widget)
			{
				widget->render();
			}
		}
	}

	void ImGuiRenderer::finalizeFrame() const
	{
		ImGui::Render();
		ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
	}

	void ImGuiRenderer::shutdown()
	{
		m_logger.info("Shutting down");

		ImGui_ImplOpenGL3_Shutdown();
		ImGui_ImplGlfw_Shutdown();
		ImGui::DestroyContext();
	}
} // namespace SE
#endif
