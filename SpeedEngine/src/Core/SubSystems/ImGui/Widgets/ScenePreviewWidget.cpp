#include "pch.h"
#include "ScenePreviewWidget.h"
#include "Engine.h"

#include "../../Graphics/Rendering/RendererManager.h"
#include <imgui.h>

namespace SE
{
	ScenePreviewWidget::ScenePreviewWidget() : Widget("ScenePreviewWidget")
	{
	}

	void ScenePreviewWidget::render() {
		_setStatus("Rendering ScenePreviewWidget");

        ImGuiWindowFlags window_flags = 0; // Default flags allow docking
        ImGui::Begin("ScenePreviewWidget", nullptr, window_flags);

		// This widget (along with other imgui widgets) receive the imgui viewport, but we want to display the scene viewport in this widget
		uint32_t fboColorId = Engine::Instance().getSubSystem<RendererManager>()->getRenderer<SceneRenderer>()->getViewport()->getFBO().getColorTexture();
		
		ImTextureID imguiTexId = static_cast<ImTextureID>(fboColorId);
		ImVec2 viewportSize = ImGui::GetContentRegionAvail();

		ImGui::Image(fboColorId, viewportSize, ImVec2(0,1),ImVec2(1,0));

        ImGui::End();
	}

	void ScenePreviewWidget::shutdown() {
		_setStatus("Shutting down ScenePreviewWidget");
	}
}
