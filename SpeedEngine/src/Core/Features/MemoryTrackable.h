#pragma once
#include "MemoryTag.h"

#define gDefineMemory(category, item) MemoryTrackable::MemoryTrackable(&::SE::tagStorage(category, item))
#define gDefineMemoryCategory(category) MemoryTrackable::MemoryTrackable(&::SE::tagStorage(category))

namespace SE
{
	class MemoryTrackable
	{
	public:
		explicit MemoryTrackable(const MemoryTag* tag);
		virtual ~MemoryTrackable() = default;

		void* operator new(std::size_t size);
		void* operator new(std::size_t size, std::align_val_t alignment);

		void operator delete(void* ptr) noexcept;
		void operator delete(void* ptr, std::size_t size) noexcept;
		void operator delete(void* ptr, std::align_val_t alignment) noexcept;
		void operator delete(void* ptr, std::size_t size, std::align_val_t alignment) noexcept;

		void* operator new[](std::size_t size);
		void* operator new[](std::size_t size, std::align_val_t alignment);

		void operator delete[](void* ptr) noexcept;
		void operator delete[](void* ptr, std::size_t size) noexcept;
		void operator delete[](void* ptr, std::align_val_t alignment) noexcept;
		void operator delete[](void* ptr, std::size_t size, std::align_val_t alignment) noexcept;

	private:
		const MemoryTag* m_memoryTag;
	};
} // namespace SE
