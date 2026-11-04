#pragma once

#include "Presentation.h"
#include "Core/ResourceManagers/Font/Font.h"
#include "Core/Scene/Entity/Component/UIRectComponent.h"
#include "Core/Scene/Entity/Component/UIImageComponent.h"
#include "Core/Scene/Entity/Component/UITextComponent.h"

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
		TextDraw(FontHandle font, std::string_view text, glm::vec2 position, glm::vec4 color,
			std::pmr::memory_resource* resource, float scale = 1.0f)
			: font(std::move(font)), text(text, resource), position(position), color(color), scale(scale)
		{}

		FontHandle font;
		std::pmr::string text;
		glm::vec2 position;
		glm::vec4 color;
		float scale;
	};

	struct UIRectDraw
	{
		UIRectDraw(const UIRectComponent& rect, glm::vec2 referenceResolution, const UIImageComponent& image,
			std::pmr::memory_resource* resource)
			: rect(rect), referenceResolution(referenceResolution), color(image.color), texturePath(image.texturePath, resource)
		{}

		UIRectComponent rect;
		glm::vec2 referenceResolution;
		glm::vec4 color;
		std::pmr::string texturePath;
	};

	struct UITextDraw
	{
		UITextDraw(const UIRectComponent& rect, glm::vec2 referenceResolution, const UITextComponent& label,
			std::pmr::memory_resource* resource)
			: rect(rect), referenceResolution(referenceResolution), text(label.text, resource), fontPath(label.fontPath, resource),
			fontPixelHeight(label.fontPixelHeight), fontSize(label.fontSize), color(label.color),
			horizontalAlignment(label.horizontalAlignment), verticalAlignment(label.verticalAlignment)
		{}

		UIRectComponent rect;
		glm::vec2 referenceResolution;
		std::pmr::string text;
		std::pmr::string fontPath;
		uint32_t fontPixelHeight;
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
			uiDraws(resource)
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
		std::pmr::vector<PointLight> lights;
		std::pmr::vector<DebugVertex> debugLines;
		std::pmr::vector<DebugVertex> debugTriangles;
		std::pmr::vector<TextDraw> textDraws;
		std::pmr::vector<UIDraw> uiDraws;
	};
} // namespace SE
