#pragma once

#include "RenderFrame.h"

namespace SE
{
	class Scene;
	class PhysicsSystem;
	struct CameraFrustum;
 
	class RenderFrameBuilder
	{
	public:
		// @brief Use before updating any text producers
		void beginSimulation() { m_textSubmissions.clear(); }

		// @brief Submits text to be rendered in the next RenderFrame
		// @brief Do not use this directly, use Engine::submitText instead, as it is not thread-safe
		// @param font The font to use for rendering the text
		// @param text The text to render
		// @param position The position in pixels from the top left
		// @param color The color of the text
		void submitText(FontHandle font, std::string_view text, glm::vec2 position, glm::vec4 color);

		// @brief Captures the current state of the scene and stores it in the builder for later
		// Call this once per simulation tick before building a RenderFrame
		// @param scene The scene to capture
		// @param tickTime The time point of the current simulation tick
		void captureSimulation(Scene& scene, std::chrono::steady_clock::time_point tickTime);

		// @brief Builds a RenderFrame from the captured state and appends debug geometry from the physics system
		// @param frame The RenderFrame to build
		// @param physics The PhysicsSystem to append debug geometry from
		void build(RenderFrame& frame, PhysicsSystem& physics) const;

	private:
		struct TextSubmission
		{
			FontHandle font;
			std::string text;
			glm::vec2 position;
			glm::vec4 color;
		};
		std::vector<TextSubmission> m_textSubmissions;

		struct DrawItemHistory
		{
			DrawItem drawItem;
			uint64_t lastSeenTick; // The last simulation tick where this entity was present
		};
		std::unordered_map<uint32_t, DrawItemHistory> m_drawItemHistories;

		std::vector<PointLight> m_lights;

		CameraSnapshot m_camera;

		const Scene* m_lastScene = nullptr;
		const CameraFrustum* m_lastCamera = nullptr;

		uint64_t m_tick = 0;
		std::chrono::steady_clock::time_point m_tickTime{};
	};
} // namespace SE
