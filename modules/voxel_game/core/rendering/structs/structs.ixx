module;

#include <glad/glad.h>
#include <GLFW/glfw3.h>

#include <cstdint>

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

	struct APIVersion {
		int8_t major;
		int8_t minor;
	};

	struct ScreenSize {
		unsigned int width;
		unsigned int height;
	};

}; // namespace vxg::core::rendering::structs