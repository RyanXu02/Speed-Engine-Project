#pragma once

#include "../SubSystems/SubSystem.h"
#include "MemoryTag.h"


namespace SE
{
	class MemoryTrackingSystem : public SubSystem
	{
		friend class Engine;
		friend class MemoryTrackable;

	public:
		void shutdown() override;
		void reportAllocations();

	private:
		MemoryTrackingSystem()
			: SubSystem("MemoryTrackingSystem")
		{}

		struct AllocationRecord
		{
			std::size_t num;
			std::size_t size;
			const MemoryTag* tag = nullptr;
		};

		void addAllocation(void* ptr, std::size_t size);
		void removeAllocation(void* ptr) noexcept;
		void setTag(const void* object, const MemoryTag* tag);

		std::mutex m_mutex;
		std::map<std::uintptr_t, AllocationRecord> m_allocations;
		std::size_t m_allocationNum = 0;
		bool m_stopped = false;
	};
} // namespace SE
