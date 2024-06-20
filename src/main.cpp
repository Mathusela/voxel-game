#include <glad/glad.h>
#include <GLFW/glfw3.h>

#include <type_traits>
#include <memory>

import voxel_game.core;
import voxel_game.exceptions;
import voxel_game.utilities;

void init_glfw() {
	if (!glfwInit())
		throw vxg::exceptions::InitError("Failed to initialise GLFW.");
}

int main() {
	try {
		// Choose rendering backend
		vxg::core::rendering::OpenGLBackend rendering_backend;

		// Create window
		std::unique_ptr<vxg::core::rendering::WindowManager> window = rendering_backend.construct_window({ {700, 500}, "Voxel Game", {4, 6} });

		// Run application
		vxg::core::App app(std::move(rendering_backend));
		return app.run(*window);
	}
	catch (const vxg::exceptions::InitError& e)
		{ return vxg::exceptions::handle_unrecoverable_error(e); }
}