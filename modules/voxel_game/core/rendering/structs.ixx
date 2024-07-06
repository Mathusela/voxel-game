module;

#include <glad/glad.h>
#include <GLFW/glfw3.h>

export module voxel_game.core.rendering.structs;

export namespace vxg::core::rendering::structs {

	struct DrawArraysIndirectCommand {
		GLuint count;
		GLuint instanceCount;
		GLuint firstVertex;
		GLuint baseInstance;
	};

	struct DrawElementsIndirectCommand {
		GLuint count;
		GLuint instanceCount;
		GLuint firstIndex;
		GLint baseVertex;
		GLuint baseInstance;
	};

}; // namespace vxg::core::rendering::structs