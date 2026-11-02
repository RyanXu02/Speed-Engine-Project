#pragma once

#include "Presentation.h"
#include "Core/ResourceManagers/Font/Font.h"

namespace SE
{
	struct DrawItem
	{
		uint32_t meshResourceId;
		uint32_t entityId;
		TransformState previous;
		TransformState current;
	};

	struct CameraSnapshot
	{
		CameraState previous;
		CameraState current;
	};

	struct DebugVertex
	{
		glm::vec3 position;
		glm::vec4 color;
	};

	struct TextDraw
	{
		TextDraw(FontHandle font, std::string_view text, glm::vec2 position, glm::vec4 color,
			std::pmr::memory_resource* resource)
			: font(std::move(font)), text(text, resource), position(position), color(color)
		{}

		FontHandle font;
		std::pmr::string text;
		glm::vec2 position;
		glm::vec4 color;
	};

	struct RenderFrame
	{
		explicit RenderFrame(std::pmr::memory_resource* resource = std::pmr::get_default_resource())
			: draws(resource), debugLines(resource), debugTriangles(resource), textDraws(resource)
		{}

		void addDebugLine(const glm::vec3& from, const glm::vec3& to, const glm::vec4& color)
		{
			debugLines.insert(debugLines.end(), { { from, color }, { to, color } });
		}

		void addDebugTriangle(const glm::vec3& a, const glm::vec3& b, const glm::vec3& c, const glm::vec4& color)
		{
			debugTriangles.insert(debugTriangles.end(), { { a, color }, { b, color }, { c, color } });
		}

		CameraSnapshot camera;
		std::chrono::steady_clock::time_point tickTime{};
		std::pmr::vector<DrawItem> draws;
		std::pmr::vector<DebugVertex> debugLines;
		std::pmr::vector<DebugVertex> debugTriangles;
		std::pmr::vector<TextDraw> textDraws;
	};
} // namespace SE
