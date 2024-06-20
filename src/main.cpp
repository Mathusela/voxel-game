#include <glad/glad.h>
#include <GLFW/glfw3.h>

#include <type_traits>
#include <memory>
#include <iostream>

import voxel_game.core;
import voxel_game.exceptions;
import voxel_game.utilities;

int main() {
	// Choose rendering backend
	auto renderingBackendConstruction = vxg::exceptions::construct_and_catch
		<vxg::core::rendering::OpenGLBackend, vxg::exceptions::InitError>(vxg::exceptions::handle_unrecoverable_error<vxg::exceptions::InitError>);
	if (renderingBackendConstruction.encounteredException)
		return renderingBackendConstruction.get_error_result();
	vxg::core::rendering::OpenGLBackend renderingBackend = std::move(renderingBackendConstruction.get_instance());

	// Create window
	std::unique_ptr<vxg::core::rendering::WindowManager> window;
	try { window = renderingBackend.construct_window({ {700, 500}, "Voxel Game", {4, 6} }); }
	catch (const vxg::exceptions::InitError& e)
		{ return vxg::exceptions::handle_unrecoverable_error(e); }

	// Run application
	vxg::core::App app(std::move(renderingBackend));
	return app.run(*window);
}