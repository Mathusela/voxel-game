module;

#include "voxel_game/reflection_macros.hpp"

#include <glm/glm.hpp>

export module voxel_game.core.structs;

import voxel_game.reflection;
import voxel_game.utilities.tmp;

export namespace vxg::core::structs {

	struct Vertex {
		glm::vec3 position;
	};

	struct ObjectData {
		glm::mat4 modelMatrix;
	};

}; // namespace vxg::core::structs

VXG_REFLECTION_DESCRIBE_CLASS(vxg::core::structs::Vertex, position);
VXG_REFLECTION_DESCRIBE_CLASS(vxg::core::structs::ObjectData, modelMatrix);