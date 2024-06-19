#include <glad/glad.h>
#include <GLFW/glfw3.h>

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

	// Create window
	vxg::core::WindowManager window;
	try { window = vxg::core::WindowManager({700, 500}, "Voxel Game", {4, 6}); }
	catch (const vxg::exceptions::InitError& e)
		{ return vxg::exceptions::handle_unrecoverable_error(e); }
	
	// Run application
	vxg::core::App app;
	return app.run(window);
}