#include <glad/glad.h>
#include <GLFW/glfw3.h>

#include <type_traits>

import voxel_game.core;
import voxel_game.exceptions;
import voxel_game.utilities;

void init_glfw() {
	if (!glfwInit())
		throw vxg::exceptions::InitError("Failed to initialise GLFW.");
}

int main() {
	// Initialize glfw
	try { init_glfw(); }
	catch (const vxg::exceptions::InitError& e)
		{ return vxg::exceptions::handle_unrecoverable_error(e); }
	vxg::utilities::DeferredFunction deferredGLFWTerminate(glfwTerminate);

	// TODO: Offload window creation to rendering backend
	// Create window
	vxg::core::WindowManager window;
	try { window = vxg::core::WindowManager({700, 500}, "Voxel Game", {4, 6}); }
	catch (const vxg::exceptions::InitError& e)
		{ return vxg::exceptions::handle_unrecoverable_error(e); }
	
	// Choose rendering backend
	vxg::core::rendering::OpenGLBackend rendering_backend;
	
	// Run application
	vxg::core::App app(std::move(rendering_backend));
	return app.run(window);
}