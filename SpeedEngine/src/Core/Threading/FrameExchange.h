#pragma once

#include "Core/Memory/FrameSlot.h"

namespace SE
{
	// @brief An exchange for RenderFrames between a producer and a consumer
	class FrameExchange
	{
	public:

		// @brief A handle that allows writing to a RenderFrame in the exchange. The handle must be published or destroyed to release the slot.
		class WriteHandle
		{
		public:
			WriteHandle(const WriteHandle&) = delete;
			WriteHandle& operator=(const WriteHandle&) = delete;
			WriteHandle(WriteHandle&& other) noexcept;
			WriteHandle& operator=(WriteHandle&&) = delete;

			~WriteHandle();

			explicit operator bool() const { return m_exchange != nullptr; }

			// @brief Get the RenderFrame for writing
			RenderFrame& frame();

			// @brief Publish the written frame, makes it available for reading, ends write ownership
			void publish();

		private:
			friend class FrameExchange;
			WriteHandle() = default;
			WriteHandle(FrameExchange& exchange, std::size_t index);

			FrameExchange* m_exchange = nullptr;
			std::size_t m_index = 0;
		};

		// @brief A handle that allows reading a RenderFrame from the exchange. The handle must be destroyed to release the slot.
		class ReadHandle
		{
		public:
			ReadHandle(const ReadHandle&) = delete;
			ReadHandle& operator=(const ReadHandle&) = delete;
			ReadHandle(ReadHandle&& other) noexcept;
			ReadHandle& operator=(ReadHandle&&) = delete;
			~ReadHandle();

			explicit operator bool() const { return m_exchange != nullptr; }
			const RenderFrame& frame() const;

		private:
			friend class FrameExchange;
			ReadHandle() = default;
			ReadHandle(FrameExchange& exchange, std::size_t index);

			FrameExchange* m_exchange = nullptr;
			std::size_t m_index = 0;
		};


		FrameExchange() = default;

		FrameExchange(const FrameExchange&) = delete;
		FrameExchange& operator=(const FrameExchange&) = delete;
		FrameExchange(FrameExchange&&) = delete;
		FrameExchange& operator=(FrameExchange&&) = delete;

		// @brief Try to begin writing a new RenderFrame. Returns an empty handle if no slot is available for writing.
		// @return A WriteHandle for the new frame, or an empty handle if no slot is available for writing.
		WriteHandle tryBeginWrite();
		// @brief Try to acquire a ready RenderFrame for reading. Returns an empty handle if no frame is ready.
		// @return A ReadHandle for the ready frame, or an empty handle if no frame is ready.
		ReadHandle tryAcquireReady();

	private:
		enum class FrameSlotState { Free, Writing, Ready, Reading };

		// @brief Publish a written frame, makes it available for reading, ends write ownership
		void _publish(std::size_t index);
		// @brief Free a slot after reading or writing, ends ownership
		void _freeSlot(std::size_t index);
		
		// Just double buffer for now, can be extended to more if we need
		std::array<FrameSlot, 2> m_slots;
		std::array<FrameSlotState, 2> m_states{ FrameSlotState::Free, FrameSlotState::Free };
		std::array<std::size_t, 2> m_ready{};

		// Head of the ready queue, points to the next ready slot to be read
		std::size_t m_readyHead = 0;
		// Number of ready slots in the queue
		std::size_t m_readyCount = 0;

		std::mutex m_mutex;
	};
} // namespace SE
