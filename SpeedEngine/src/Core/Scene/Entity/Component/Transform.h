#pragma once
#include "Features/Serializable.h"

#include <glm/glm.hpp>
#include <glm/gtc/quaternion.hpp>
#include <nlohmann/json.hpp>


namespace glm
{
	inline void to_json(nlohmann::json& j, const glm::vec2& v)
	{
		j = nlohmann::json{ v.x, v.y };
	}

	inline void from_json(const nlohmann::json& j, glm::vec2& v)
	{
		j.at(0).get_to(v.x);
		j.at(1).get_to(v.y);
	}

	inline void to_json(nlohmann::json& j, const glm::vec3& v)
	{
		j = nlohmann::json{ v.x, v.y, v.z };
	}

	inline void from_json(const nlohmann::json& j, glm::vec3& v)
	{
		j.at(0).get_to(v.x);
		j.at(1).get_to(v.y);
		j.at(2).get_to(v.z);
	}

	inline void to_json(nlohmann::json& j, const glm::vec4& v)
	{
		j = nlohmann::json{ v.x, v.y, v.z, v.w };
	}

	inline void from_json(const nlohmann::json& j, glm::vec4& v)
	{
		j.at(0).get_to(v.x);
		j.at(1).get_to(v.y);
		j.at(2).get_to(v.z);
		j.at(3).get_to(v.w);
	}

	inline void to_json(nlohmann::json& j, const glm::quat& q)
	{
		j = nlohmann::json{ q.x, q.y, q.z, q.w };
	}

	inline void from_json(const nlohmann::json& j, glm::quat& q)
	{
		j.at(0).get_to(q.x);
		j.at(1).get_to(q.y);
		j.at(2).get_to(q.z);
		j.at(3).get_to(q.w);
	}

	inline void to_json(nlohmann::json& j, const glm::mat4& m)
	{
		j = nlohmann::json{ m[0], m[1], m[2], m[3] };
	}

	inline void from_json(const nlohmann::json& j, glm::mat4& m)
	{
		j.at(0).get_to(m[0]);
		j.at(1).get_to(m[1]);
		j.at(2).get_to(m[2]);
		j.at(3).get_to(m[3]);
	}
}

namespace SE
{
	// @brief Represents a transform component that can be attached to an entity
	struct Transform
	{
		glm::vec3 translate{ 0.0f, 0.0f, 0.0f };
		glm::quat rotate{ 1.0f, 0.0f, 0.0f, 0.0f };
		glm::vec3 scale{ 1.0f, 1.0f, 1.0f };

		DEFINE_TYPE(Transform,
			member("translate", &Transform::translate),
			member("rotate", &Transform::rotate),
			member("scale", &Transform::scale)
		);
	};
}
