module;

#include <glm/glm.hpp>

export module voxel_game.core.rendering:structs;

export namespace vxg::core::rendering {

	struct Vertex {
		glm::vec3 position;
	};

}; // namespace vxg::core::rendering