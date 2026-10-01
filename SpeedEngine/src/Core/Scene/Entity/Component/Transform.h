#pragma once

#include <glm/glm.hpp>
#include <nlohmann/json.hpp>
#include <string>


namespace glm
{
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
	class Transform
	{
	public:
		Transform() = default;

		glm::vec3 translate{ 0.0f, 0.0f, 0.0f };
		glm::vec3 rotate{ 0.0f, 0.0f, 0.0f };
		glm::vec3 scale{ 1.0f, 1.0f, 1.0f };

		void setPosition(const glm::vec3& p)
		{
			translate = p;
			m_isDirty = true;
		}
		void setRotation(const glm::vec3& r)
		{
			rotate = r;
			m_isDirty = true;
		}
		void setScale(const glm::vec3& s)
		{
			scale = s;
			m_isDirty = true;
		}

		const glm::vec3& getTranslation() const { return translate; }
		const glm::vec3& getRotation() const { return rotate; }
		const glm::vec3& getScale() const { return scale; }
		const glm::mat4& getTransform();

		NLOHMANN_DEFINE_TYPE_INTRUSIVE(Transform, translate, rotate, scale);

	private:
		bool m_scaleLink{ true };

		bool m_isDirty{ true };
		glm::mat4 m_transform{ 1.0f };
	};
}
