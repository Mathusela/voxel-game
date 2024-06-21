#include <glad/glad.h>
#include <GLFW/glfw3.h>

#include <utility>
#include <exception>

import voxel_game.core;
import voxel_game.exceptions;
import voxel_game.utilities;

int main() {
	// Construct rendering context
	using Backend = vxg::core::rendering::OpenGLBackend;
	using Context = vxg::core::rendering::RenderingContext<Backend>;

	auto contextConstructionResult = vxg::exceptions::construct_and_catch<Context, std::exception>(
		vxg::exceptions::handle_unrecoverable_error<std::exception>,
		vxg::core::rendering::WindowProperties { {700, 500}, "Voxel Game", {4, 6} }
	);

	// Handle errors
	if (contextConstructionResult.encounteredException)
		return contextConstructionResult.get_error_result();
	Context renderingContext = std::move(contextConstructionResult.get_instance());

	// Run application
	vxg::core::App app(std::move(renderingContext));
	return app.run();
}