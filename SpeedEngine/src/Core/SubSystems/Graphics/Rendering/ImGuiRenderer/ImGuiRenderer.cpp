#include "pch.h"
#include "ImGuiRenderer.h"

#include "imgui.h"
#include "imgui_impl_glfw.h"
#include "imgui_impl_opengl3.h"

#include "../../Window.h"

#include "../../../ImGui/Widget.h"
#include "../../../ImGui/Widgets/TestWidget.h"
#include "../../../ImGui/Widgets/MainWidget.h"
#include "../../../ImGui/Widgets/ResourceBrowserWidget.h"

namespace SE
{
	void ImGuiRenderer::init()
	{
		m_logger.info("Initializing");

		IMGUI_CHECKVERSION();
		ImGui::CreateContext();
		ImGuiIO& io = ImGui::GetIO();
		io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;     // Enable Keyboard Controls
		io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;      // Enable Gamepad Controls
		io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;         // IF using Docking Branch

		// Setup Platform/Renderer backends
		ImGui_ImplGlfw_InitForOpenGL(m_window->getWindow(), true);          // Second param install_callback=true will install GLFW callbacks and chain to existing ones.
		ImGui_ImplOpenGL3_Init();

		initWidgets();


	}

	void ImGuiRenderer::initWidgets()
	{
		createWidget<MainWidget>();
		createWidget<ResourceBrowserWidget>();
		// ...
	}

	void ImGuiRenderer::update(double deltaTime)
	{
		ImGui_ImplOpenGL3_NewFrame();
		ImGui_ImplGlfw_NewFrame();
		ImGui::NewFrame();
		ImGui::ShowDemoWindow(); // Show demo window! :)
	}

	void ImGuiRenderer::render(Viewport& viewport) const
	{
		for (const auto& [id, widget] : m_widgets)
		{
			if (widget)
			{
				widget->render();
			}
		}

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
}
