#include "pch.h"

#if defined(COMPILE_WITH_EDITOR)
#include "SubSystems/ImGui/Widgets/EngineStateWidget.h"
#include "SubSystems/ImGui/Widgets/MainWidget.h"
#include "SubSystems/ImGui/Widgets/MemoryWidget.h"
#include "SubSystems/ImGui/Widgets/PropertiesWidget.h"
#include "SubSystems/ImGui/Widgets/ResourceBrowserWidget.h"
#include "SubSystems/ImGui/Widgets/SceneInfoWidget.h"
#include "SubSystems/ImGui/Widgets/ScenePreviewWidget.h"
#include "Core/SubSystems/Scenes/SceneSystem.h"
#include "Engine.h"
#endif

#include "Core/SubSystems/ImGui/Widgets/ConsoleWidget.h"
#include "SubSystems/Graphics/Window.h"
#include "ImGuiRenderer.h"

#include <imgui.h>
#include <imgui_impl_glfw.h>
#include <imgui_impl_opengl3.h>

namespace SE
{
	inline void SetDarkPastelImGuiStyle()
	{
		ImGuiStyle& style = ImGui::GetStyle();
		style.WindowMenuButtonPosition = ImGuiDir_None; // hide collapse button

		style.FontSizeBase = 4.0f;
		style.FontScaleDpi = 4.0f;

		ImVec4* colors = style.Colors;

		// Backgrounds
		colors[ImGuiCol_WindowBg] = ImVec4(0.12f, 0.13f, 0.15f, 1.00f); // Dark grey base
		colors[ImGuiCol_ChildBg] = ImVec4(0.14f, 0.15f, 0.17f, 1.00f);
		colors[ImGuiCol_PopupBg] = ImVec4(0.10f, 0.10f, 0.12f, 0.95f);
		colors[ImGuiCol_Border] = ImVec4(0.30f, 0.33f, 0.42f, 0.40f);

		//Text
		colors[ImGuiCol_Text] = ImVec4(0.90f, 0.93f, 0.95f, 1.00f);
		colors[ImGuiCol_TextDisabled] = ImVec4(0.60f, 0.65f, 0.70f, 1.00f);

		// Headers
		colors[ImGuiCol_Header] = ImVec4(0.36f, 0.42f, 0.55f, 0.60f);
		colors[ImGuiCol_HeaderHovered] = ImVec4(0.44f, 0.50f, 0.68f, 0.80f);
		colors[ImGuiCol_HeaderActive] = ImVec4(0.46f, 0.55f, 0.75f, 1.00f);

		// Buttons
		colors[ImGuiCol_Button] = ImVec4(0.28f, 0.34f, 0.48f, 0.70f);
		colors[ImGuiCol_ButtonHovered] = ImVec4(0.36f, 0.45f, 0.65f, 0.85f);
		colors[ImGuiCol_ButtonActive] = ImVec4(0.40f, 0.50f, 0.70f, 1.00f);

		// Frames
		colors[ImGuiCol_FrameBg] = ImVec4(0.20f, 0.22f, 0.28f, 1.00f);
		colors[ImGuiCol_FrameBgHovered] = ImVec4(0.28f, 0.32f, 0.42f, 1.00f);
		colors[ImGuiCol_FrameBgActive] = ImVec4(0.32f, 0.38f, 0.50f, 1.00f);

		// Tabs
		colors[ImGuiCol_Tab] = ImVec4(0.26f, 0.30f, 0.42f, 0.80f);
		colors[ImGuiCol_TabHovered] = ImVec4(0.36f, 0.42f, 0.58f, 1.00f);
		colors[ImGuiCol_TabActive] = ImVec4(0.42f, 0.50f, 0.68f, 1.00f);
		colors[ImGuiCol_TabUnfocused] = ImVec4(0.20f, 0.24f, 0.32f, 0.80f);
		colors[ImGuiCol_TabUnfocusedActive] = ImVec4(0.30f, 0.36f, 0.50f, 1.00f);

		// Titles
		colors[ImGuiCol_TitleBg] = ImVec4(0.20f, 0.25f, 0.30f, 1.00f);
		colors[ImGuiCol_TitleBgActive] = ImVec4(0.25f, 0.30f, 0.40f, 1.00f);
		colors[ImGuiCol_TitleBgCollapsed] = ImVec4(0.10f, 0.12f, 0.15f, 0.75f);

		// Scrollbars
		colors[ImGuiCol_ScrollbarBg] = ImVec4(0.13f, 0.14f, 0.18f, 1.00f);
		colors[ImGuiCol_ScrollbarGrab] = ImVec4(0.25f, 0.30f, 0.38f, 0.60f);
		colors[ImGuiCol_ScrollbarGrabHovered] = ImVec4(0.35f, 0.40f, 0.50f, 0.80f);
		colors[ImGuiCol_ScrollbarGrabActive] = ImVec4(0.45f, 0.50f, 0.65f, 1.00f);

		// Checkboxes / Radios
		colors[ImGuiCol_CheckMark] = ImVec4(0.80f, 0.85f, 1.00f, 1.00f);

		// Sliders
		colors[ImGuiCol_SliderGrab] = ImVec4(0.50f, 0.65f, 0.90f, 1.00f);
		colors[ImGuiCol_SliderGrabActive] = ImVec4(0.60f, 0.75f, 1.00f, 1.00f);

		// Resize Grip
		colors[ImGuiCol_ResizeGrip] = ImVec4(0.30f, 0.40f, 0.50f, 0.60f);
		colors[ImGuiCol_ResizeGripHovered] = ImVec4(0.40f, 0.50f, 0.60f, 0.80f);
		colors[ImGuiCol_ResizeGripActive] = ImVec4(0.50f, 0.60f, 0.80f, 1.00f);

		// Separator
		colors[ImGuiCol_Separator] = ImVec4(0.35f, 0.40f, 0.48f, 0.7f);
		colors[ImGuiCol_SeparatorHovered] = ImVec4(0.50f, 0.60f, 0.72f, 0.9f);
		colors[ImGuiCol_SeparatorActive] = ImVec4(0.65f, 0.70f, 0.85f, 1.0f);

		// Menus and Tooltips
		colors[ImGuiCol_MenuBarBg] = ImVec4(0.14f, 0.15f, 0.17f, 1.00f);
		// colors[ImGuiCol_TooltipBg]          = ImVec4(0.18f, 0.20f, 0.25f, 0.95f);

		// Drag & Drop
		colors[ImGuiCol_DragDropTarget] = ImVec4(0.50f, 0.85f, 1.00f, 0.90f);

		// Style Metrics
		style.WindowRounding = 8.0f;
		style.ChildRounding = 6.0f;
		style.FrameRounding = 5.0f;
		style.PopupRounding = 6.0f;
		style.ScrollbarRounding = 5.0f;
		style.GrabRounding = 4.0f;
		style.TabRounding = 5.0f;

		style.WindowBorderSize = 0.0f;
		style.FrameBorderSize = 0.0f;
		style.PopupBorderSize = 1.0f;

		style.WindowPadding = ImVec2(16, 16);
		style.FramePadding = ImVec2(10, 6);
		style.ItemSpacing = ImVec2(10, 10);
		style.ItemInnerSpacing = ImVec2(6, 4);
		style.IndentSpacing = 20.0f;
	}


	void ImGuiRenderer::init()
	{
		m_logger.info("Initializing");

		IMGUI_CHECKVERSION();
		ImGui::CreateContext();
		ImGuiIO& io = ImGui::GetIO();

		auto* font = io.Fonts->AddFontFromFileTTF("fonts/Quicksand-SemiBold.ttf");
		io.FontDefault = font;

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

		SetDarkPastelImGuiStyle();

		initWidgets();
	}

	void ImGuiRenderer::initWidgets()
	{
#if defined(COMPILE_WITH_EDITOR)
		m_mainWidgetId = createWidget<MainWidget>(m_session);
		createWidget<ResourceBrowserWidget>();
		createWidget<MemoryWidget>();
		createWidget<ScenePreviewWidget>();
		createWidget<SceneInfoWidget>(m_session);
		createWidget<PropertiesWidget>(m_session);
		createWidget<EngineStateWidget>();
#endif
		createWidget<ConsoleWidget>();
		// ...
	}

	void ImGuiRenderer::beginFrame()
	{
		ImGui_ImplOpenGL3_NewFrame();
		ImGui_ImplGlfw_NewFrame();
		ImGui::NewFrame();
		// ImGui::ShowDemoWindow(); // Don't show demo window! :(
	}

	void ImGuiRenderer::render(RenderContext& context)
	{
#if defined(COMPILE_WITH_EDITOR)
		auto* scenes = Engine::Instance().getSubSystem<SceneSystem>();
		m_session.syncSceneRevision(scenes->getSceneRevision());
		auto* scene = scenes->getCurrentScene();
		for (auto it = m_session.selectedEntities().begin(); it != m_session.selectedEntities().end();)
		{
			const uint32_t id = *it++;
			if (!scene || !scene->hasEntity(id) || scene->isEntityPendingRemoval(id))
			{
				m_session.removeSelected(id);
				continue;
			}
			const auto* life = scene->getEntity(id).getComponent<LifeComponent>();
			if (life && !life->isAlive)
				m_session.removeSelected(id);
		}
		auto* main = getWidget<MainWidget>(m_mainWidgetId);
		main->beginEditorFrame();
		main->render();
#endif

		for (const auto& [id, widget] : m_widgets)
		{
#if defined(COMPILE_WITH_EDITOR)
			if (id == m_mainWidgetId)
				continue;
#endif
			if (widget)
			{
				widget->render();
			}
		}

#if defined(COMPILE_WITH_EDITOR)
		main->finishEditorFrame();
#endif
	}


	bool ImGuiRenderer::isBlockingGameInput(bool gameItemActive) const
	{
#if defined(COMPILE_WITH_EDITOR)
		const bool blocked = MainWidget::isSceneEditingBlocked();
#else
		const bool blocked = ImGui::GetIO().WantCaptureMouse;
#endif

		return blocked || (ImGui::IsAnyItemActive() && !gameItemActive) ||
			ImGui::IsPopupOpen(nullptr, ImGuiPopupFlags_AnyPopupId | ImGuiPopupFlags_AnyPopupLevel);
	}

	void ImGuiRenderer::finalizeFrame() const
	{
		ImGui::Render();
		ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
	}

	void ImGuiRenderer::shutdown()
	{
		m_logger.info("Shutting down");

		for (auto& [id, widget] : m_widgets)
		{
			if (widget)
			{
				widget->shutdown();
			}
		}
		m_widgets.clear();

		ImGui_ImplOpenGL3_Shutdown();
		ImGui_ImplGlfw_Shutdown();
		ImGui::DestroyContext();
	}
} // namespace SE
