#pragma once

#include <string>
#include <nlohmann/json.hpp>
#include "core/features/math.h"

namespace SE {
	struct NameComponent {
		std::string name;

		NLOHMANN_DEFINE_TYPE_INTRUSIVE(NameComponent, name)
	};

	struct Mesh {
	
	};

	struct Transform{
		glm::vec3 translate{};
		glm::vec3 rotate{};
		glm::vec3 scale{};
	};

}

