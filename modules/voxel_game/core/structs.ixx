module;

#include <glm/glm.hpp>

export module voxel_game.core.structs;

export namespace vxg::core::structs {

	struct Vertex {
		glm::vec3 position;
	};

	struct ObjectData {
		glm::vec3 position;
	};

}; // namespace vxg::core::structs