#pragma once

#include "Core/Features/CameraState.h"

namespace SE
{
	struct DrawItem
	{
		uint32_t meshResourceId;
		uint32_t entityId;
		glm::mat4 modelMatrix;
	};

	struct DebugVertex
	{
		glm::vec3 position;
		glm::vec4 color;
	};

	struct RenderFrame
	{
		explicit RenderFrame(std::pmr::memory_resource* resource = std::pmr::get_default_resource())
			: draws(resource), debugLines(resource), debugTriangles(resource)
		{}

		void addDebugLine(const glm::vec3& from, const glm::vec3& to, const glm::vec4& color)
		{
			debugLines.insert(debugLines.end(), { { from, color }, { to, color } });
		}

		void addDebugTriangle(const glm::vec3& a, const glm::vec3& b, const glm::vec3& c, const glm::vec4& color)
		{
			debugTriangles.insert(debugTriangles.end(), { { a, color }, { b, color }, { c, color } });
		}

		CameraState camera;
		std::pmr::vector<DrawItem> draws;
		std::pmr::vector<DebugVertex> debugLines;
		std::pmr::vector<DebugVertex> debugTriangles;
	};
} // namespace SE
