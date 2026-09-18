#pragma once

#include "Transform.h"

#include <nlohmann/json.hpp>
#include <string>

namespace SE
{
	struct NameComponent
	{
		std::string name;

		NLOHMANN_DEFINE_TYPE_INTRUSIVE(NameComponent, name)
	};

	struct MeshComponent
	{
		MeshComponent() = default;
		MeshComponent(uint32_t meshId)
		    : meshResourceID(meshId)
		{}

		uint32_t meshResourceID{};
	};

	struct LifeComponent
	{
		LifeComponent() = default;
		LifeComponent(bool alive)
		    : isAlive(alive)
		{}

		bool isAlive{ true };
	};

}
