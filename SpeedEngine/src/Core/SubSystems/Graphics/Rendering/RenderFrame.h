#pragma once

#include "Presentation.h"
#include "Core/ResourceManagers/Resource.h"
#include "Core/Scene/Entity/Component/UIRectComponent.h"
#include "Core/Scene/Entity/Component/UIImageComponent.h"
#include "Core/Scene/Entity/Component/UITextComponent.h"

namespace SE
{
	struct DrawItem
	{
		ResourceID meshResourceId;
		ResourceID terrainResourceId;
		uint32_t entityId;
		TransformState previous;
		TransformState current;
	};

	struct CameraSnapshot
	{
		CameraState previous;
		CameraState current;
	};

	struct PointLight
	{
		glm::vec3 position;
		glm::vec3 radiance; // color * intensity
	};

	struct DebugVertex
	{
		glm::vec3 position;
		glm::vec4 color;
	};

	struct TextDraw
	{
		TextDraw(ResourceID font, std::string_view text, glm::vec2 position, glm::vec4 color,
			std::pmr::memory_resource* resource, float scale = 1.0f)
			: font(font), text(text, resource), position(position), color(color), scale(scale)
		{}

		ResourceID font;
		std::pmr::string text;
		glm::vec2 position;
		glm::vec4 color;
		float scale;
	};

	struct UIRectDraw
	{
		UIRectDraw(const UIRectComponent& rect, glm::vec2 referenceResolution, const UIImageComponent& image,
			ResourceID texture)
			: rect(rect), referenceResolution(referenceResolution), color(image.color), texture(texture)
		{}

		UIRectComponent rect;
		glm::vec2 referenceResolution;
		glm::vec4 color;
		ResourceID texture;
	};

	//billboard particle
	struct ParticleInstance
	{
		glm::vec3 position;
		glm::vec2 size;
		float rotation; // radians, around the view axis
		glm::vec4 color;
	};

	struct UITextDraw
	{
		UITextDraw(const UIRectComponent& rect, glm::vec2 referenceResolution, const UITextComponent& label,
			ResourceID font, std::pmr::memory_resource* resource)
			: rect(rect), referenceResolution(referenceResolution), text(label.text, resource), font(font),
			fontSize(label.fontSize), color(label.color),
			horizontalAlignment(label.horizontalAlignment), verticalAlignment(label.verticalAlignment)
		{}

		UIRectComponent rect;
		glm::vec2 referenceResolution;
		std::pmr::string text;
		ResourceID font;
		float fontSize;
		glm::vec4 color;
		UIHorizontalAlignment horizontalAlignment;
		UIVerticalAlignment verticalAlignment;
	};

	using UIDraw = std::variant<UIRectDraw, UITextDraw>;

	struct RenderFrame
	{
		explicit RenderFrame(std::pmr::memory_resource* resource = std::pmr::get_default_resource())
			: draws(resource),
			lights(resource),
			debugLines(resource),
			debugTriangles(resource),
			textDraws(resource),
			uiDraws(resource),
			particles(resource)
		{}

		void addDebugLine(const glm::vec3& from, const glm::vec3& to, const glm::vec4& color)
		{
			debugLines.push_back({ from, color });
			debugLines.push_back({ to, color });
		}

		void addDebugTriangle(const glm::vec3& a, const glm::vec3& b, const glm::vec3& c, const glm::vec4& color)
		{
			debugTriangles.push_back({ a, color });
			debugTriangles.push_back({ b, color });
			debugTriangles.push_back({ c, color });
		}

		CameraSnapshot camera;
		std::chrono::steady_clock::time_point tickTime{};
		std::pmr::vector<DrawItem> draws;
		std::pmr::vector<PointLight> lights;
		std::pmr::vector<DebugVertex> debugLines;
		std::pmr::vector<DebugVertex> debugTriangles;
		std::pmr::vector<TextDraw> textDraws;
		std::pmr::vector<ParticleInstance> particles; // billboards, sorted back to front
		std::pmr::vector<UIDraw> uiDraws;
	};
} // namespace SE
