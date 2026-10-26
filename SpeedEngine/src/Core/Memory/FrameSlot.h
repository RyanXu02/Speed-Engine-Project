#pragma once

#include "Core/SubSystems/Graphics/Rendering/RenderFrame.h"

namespace SE
{
	class FrameSlot
	{
	public:
		static constexpr std::size_t InitialCapacity = 2 * 1024 * 1024;

		explicit FrameSlot(std::size_t initialCapacity = InitialCapacity);

		FrameSlot(const FrameSlot&) = delete;
		FrameSlot& operator=(const FrameSlot&) = delete;
		FrameSlot(FrameSlot&&) = delete;
		FrameSlot& operator=(FrameSlot&&) = delete;

		// @brief Call this at the beginning of a new frame to reset the slot's memory and prepare for writing a new RenderFrame
		void beginFrame();

		RenderFrame& frame() { return *m_frame; }
		const RenderFrame& frame() const { return *m_frame; }

	private:
		std::unique_ptr<std::byte[]> m_storage;
		std::pmr::monotonic_buffer_resource m_arena;
		std::optional<RenderFrame> m_frame;
	};
} // namespace SE
