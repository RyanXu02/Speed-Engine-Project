#include "pch.h"

#include "MemoryTrackingSystem.h"
#include "../Logger/Logger.h"


namespace SE
{
	void MemoryTrackingSystem::shutdown()
	{
		{
			std::lock_guard lock(m_mutex);
			m_stopped = true;
		}
		reportAllocations();
		{
			std::lock_guard lock(m_mutex);
			assert(m_allocations.empty() && "Tracked allocations remain at shutdown");
		}
		SubSystem::shutdown();
	}

	void MemoryTrackingSystem::addAllocation(void* ptr, std::size_t size)
	{
		std::lock_guard lock(m_mutex);
		if (m_stopped)
			throw std::logic_error("MemoryTrackingSystem has shut down");
		auto [it, inserted] = m_allocations.emplace(reinterpret_cast<std::uintptr_t>(ptr), AllocationRecord{ m_allocationNum, size });
		if (!inserted)
			throw std::logic_error("Duplicate tracked allocation");
		++m_allocationNum;
	}

	void MemoryTrackingSystem::removeAllocation(void* ptr) noexcept
	{
		std::lock_guard lock(m_mutex);
		m_allocations.erase(reinterpret_cast<std::uintptr_t>(ptr));
	}

	void MemoryTrackingSystem::setTag(const void* object, const MemoryTag* tag)
	{
		const auto address = reinterpret_cast<std::uintptr_t>(object);
		std::lock_guard lock(m_mutex);
		auto it = m_allocations.upper_bound(address);
		if (it == m_allocations.begin())
			return;
		--it;
		if (address - it->first < it->second.size)
			it->second.tag = tag;
	}

	void MemoryTrackingSystem::reportAllocations()
	{
		if (!m_logger)
			return;

		std::map<std::uintptr_t, AllocationRecord> allocations;
		{
			std::lock_guard lock(m_mutex);
			allocations = m_allocations;
		}

		std::unordered_map<std::string, std::vector<std::pair<void*, AllocationRecord>>> categories;
		for (const auto& [address, record] : allocations)
		{
			categories[record.tag ? record.tag->category : "Unknown"].emplace_back(reinterpret_cast<void*>(address), record);
		}

		m_logger->info("Current Memory Allocations (by Category):");
		std::size_t totalMemoryUsage = 0;
		for (const auto& [category, records] : categories)
		{
			std::size_t categoryMemoryUsage = 0;
			m_logger->info("{}:", category);
			for (const auto& [ptr, record] : records)
			{
				categoryMemoryUsage += record.size;
				m_logger->info("\t{:-<28.28}>Address: {:>14p}, Size: {:>4} bytes, Num: {:>4}",
					(record.tag ? record.tag->item : "Unknown") + std::string(" "), ptr, record.size, record.num);
			}
			totalMemoryUsage += categoryMemoryUsage;
			m_logger->info("\tTotal Memory Usage for {}: {} bytes", category, categoryMemoryUsage);
		}
		m_logger->info("Total Memory Usage: {} bytes", totalMemoryUsage);
	}
} // namespace SE
