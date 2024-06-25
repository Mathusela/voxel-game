module;

#include<glm/glm.hpp>

#include <cstdint>

export module voxel_game.utilities:glm_helpers;

export namespace vxg::utilities {

	template <typename T> consteval uint8_t num_components();
	template <> consteval uint8_t num_components<glm::vec1>() { return 1; }
	template <> consteval uint8_t num_components<glm::uvec1>() { return 1; }
	template <> consteval uint8_t num_components<glm::vec2>() { return 2; }
	template <> consteval uint8_t num_components<glm::uvec2>() { return 2; }
	template <> consteval uint8_t num_components<glm::vec3>() { return 3; }
	template <> consteval uint8_t num_components<glm::uvec3>() { return 3; }
	template <> consteval uint8_t num_components<glm::vec4>() { return 4; }
	template <> consteval uint8_t num_components<glm::uvec4>() { return 4; }

};	// namespace vxg::utilities

