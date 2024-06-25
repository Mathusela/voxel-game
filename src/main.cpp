#include <utility>
#include <exception>
#include <cstdint>

import voxel_game.core;
import voxel_game.exceptions;
import voxel_game.utilities;

// TODO: Resizing + minimum free size (max size of chunk)
// TODO: Refactor deallocation merging code
// TODO: Remove the Testing things from opengl_backend.ixx

int main() {
	// Construct rendering context
	using Backend = vxg::core::rendering::OpenGLBackend;
	using Context = vxg::core::rendering::RenderingContext<Backend>;

	auto contextConstructionResult = vxg::exceptions::construct_and_catch<Context, std::exception>(
		vxg::exceptions::handle_unrecoverable_error<std::exception>,
		vxg::core::rendering::WindowProperties{ {700, 500}, "Voxel Game", {4, 6} },
		static_cast<uint16_t>(3),
		static_cast<uint_fast32_t>(100),
		static_cast<uint_fast32_t>(10)
	);

	// Handle errors
	if (contextConstructionResult.encounteredException)
		return contextConstructionResult.get_error_result();
	Context renderingContext = std::move(contextConstructionResult.get_instance());

	// Run application
	vxg::core::App app(std::move(renderingContext));
	return app.run();
}