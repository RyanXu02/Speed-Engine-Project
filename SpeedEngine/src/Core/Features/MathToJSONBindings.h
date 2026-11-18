#pragma once

#include <glm/glm.hpp>
#include <glm/gtc/quaternion.hpp>
#include <nlohmann/json.hpp>

namespace glm
{
	// All glm vec types (float, double, int, uint, bool)
	template<length_t L, typename T, qualifier Q>
	inline void to_json(nlohmann::json& j, const vec<L, T, Q>& v)
	{
		j = nlohmann::json::array();
		for (length_t i = 0; i < L; ++i)
			j.push_back(v[i]);
	}
	// All glm vec types (float, double, int, uint, bool)
	template<length_t L, typename T, qualifier Q>
	inline void from_json(const nlohmann::json& j, vec<L, T, Q>& v)
	{
		for (length_t i = 0; i < L; ++i)
			j.at(i).get_to(v[i]);
	}


	// all glm mat types (2x2, 3x3, 4x4)
	template<length_t L, qualifier Q>
	inline void to_json(nlohmann::json& j, const mat<L, L, float, Q>& m)
	{
		j = nlohmann::json::array();
		for (length_t i = 0; i < L; ++i)
			j.push_back(m[i]);
	}
	// all glm mat types (2x2, 3x3, 4x4)
	template<length_t L, qualifier Q>
	inline void from_json(const nlohmann::json& j, mat<L, L, float, Q>& m)
	{
		for (length_t i = 0; i < L; ++i)
			j.at(i).get_to(m[i]);
	}


	// glm quat type
	inline void to_json(nlohmann::json& j, const quat& q)
	{
		j = nlohmann::json{ q.x, q.y, q.z, q.w };
	}
	// glm quat type
	inline void from_json(const nlohmann::json& j, quat& q)
	{
		j.at(0).get_to(q.x);
		j.at(1).get_to(q.y);
		j.at(2).get_to(q.z);
		j.at(3).get_to(q.w);
	}
}
