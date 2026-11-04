#pragma once

#include "../SubSystems/SubSystem.h"
#include "MemoryTag.h"

#include <cstdint>
#include <map>
#include <mutex>
#include <string>
#include <unordered_map>
#include <vector>


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

	public:
		// Snapshot of memory tracker suitable for UI or external reporting
		struct AllocationInfo
		{
			void* address = nullptr;
			std::size_t size = 0;
			std::size_t num = 0;
			std::string category;
			std::string item;
		};

		struct Snapshot
		{
			std::size_t currentBytes = 0;
			std::size_t peakBytes = 0;
			std::size_t totalAllocations = 0;
			std::size_t totalDeallocations = 0;
			std::unordered_map<std::string, std::vector<AllocationInfo>> categories;
		};

		// Returns a copy of current tracking state. Thread-safe.
		Snapshot getSnapshot();

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

		// statistics
		std::size_t m_totalAllocatedBytes = 0; // current live bytes
		std::size_t m_peakAllocatedBytes = 0;
		std::size_t m_totalAllocations = 0; // total allocation calls
		std::size_t m_totalDeallocations = 0;
	};
} // namespace SE

// Inline snapshot implementation (thread-safe) to allow UI access without
// requiring changes to the .cpp implementation.
inline SE::MemoryTrackingSystem::Snapshot SE::MemoryTrackingSystem::getSnapshot()
{
	Snapshot snap;

	std::map<std::uintptr_t, AllocationRecord> allocations;
	{
		std::lock_guard lock(m_mutex);
		allocations = m_allocations;
		snap.currentBytes = m_totalAllocatedBytes;
		snap.peakBytes = m_peakAllocatedBytes;
		snap.totalAllocations = m_totalAllocations;
		snap.totalDeallocations = m_totalDeallocations;
	}

	for (const auto& [address, record] : allocations)
	{
		AllocationInfo info;
		info.address = reinterpret_cast<void*>(address);
		info.size = record.size;
		info.num = record.num;
		if (record.tag)
		{
			info.category = record.tag->category;
			info.item = record.tag->item;
		}
		else
		{
			info.category = "Unknown";
			info.item = "Unknown";
		}
		snap.categories[info.category].push_back(info);
	}

	return snap;
}
