#pragma once
#include "../SubSystems/SubSystem.h"

namespace SE
{
	// baseclass
	class ResourceManager : public SubSystem
	{
	public:
		ResourceManager() : SubSystem("ResourceManager") {}

		void update(double deltaTime) override {};

		uint32_t generateId();

	private:
		std::atomic<uint32_t> m_ids{ 1 }; // 0 for invalid id
	};
}