#pragma once

#include "Transform.h"


namespace SE
{
	struct NameComponent
	{
		NameComponent() = default;
		NameComponent(std::string_view n)
		    : name(n){};

		std::string name = "Unknown";

		NLOHMANN_DEFINE_TYPE_INTRUSIVE(NameComponent, name);
	};

	struct MeshComponent
	{
		MeshComponent() = default;
		MeshComponent(std::string p)
		    : path(p){};
		MeshComponent(uint32_t meshId)
		    : meshResourceID(meshId)
		{}

		uint32_t meshResourceID{ 0 };
		std::string path = "None";

		NLOHMANN_DEFINE_TYPE_INTRUSIVE(MeshComponent, path);
	};

	struct LifeComponent
	{
		LifeComponent() = default;
		LifeComponent(bool alive)
		    : isAlive(alive)
		{}

		bool isAlive{ true };

		NLOHMANN_DEFINE_TYPE_INTRUSIVE(LifeComponent, isAlive);
	};

	struct RelationshipComponent
	{
		RelationshipComponent() = default;

		uint32_t parent;
		std::vector<uint32_t> children;
	};

	struct PrefabComponent
	{
		PrefabComponent() = default;

		std::string prefabPath = "None";

		NLOHMANN_DEFINE_TYPE_INTRUSIVE(PrefabComponent, prefabPath);
	};

}
