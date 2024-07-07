#include <utility>
#include <exception>
#include <cstdint>
#include <iostream>

import voxel_game.core;
import voxel_game.exceptions;
import voxel_game.utilities;

import voxel_game.logging;

// NOTE: Module local definitions are still namespaced due to MSVC tooling limitations leading to linker errors

// TODO: If doing GPU meshing and allocation without returning to CPU minimum free size (max size of chunk)
// TODO: Bindless textures - load all textures into VRAM for whole program lifetime (bindless avoids texture binding point limits)
// TODO: Define NDEBUG for release builds
// TODO: Rename raw AllocationIdentifier struct in opengl_allocator to OpenGLAllocationIdentifier
// TODO: Add log levels/filtering
// TODO: Make logging thread-safe
// TODO: Flush log file during program runtime

int main() {
	// Construct rendering context
	using Allocator = vxg::core::memory::OpenGLAllocator;
	using Backend = vxg::core::rendering::OpenGLBackend<Allocator>;
	using Context = vxg::core::rendering::RenderingContext<Backend>;

	auto contextConstructionResult = vxg::exceptions::construct_and_catch<Context, std::exception>(
		vxg::exceptions::handle_unrecoverable_error<std::exception>,
		vxg::core::rendering::WindowProperties{ {700, 500}, "Voxel Game", {4, 6} },
		static_cast<uint16_t>(3),
		static_cast<size_t>(300),
		static_cast<size_t>(100),
		static_cast<size_t>(100)
	);

	// Handle errors
	if (contextConstructionResult.encounteredException)
		return contextConstructionResult.get_error_result();
	Context renderingContext = std::move(contextConstructionResult.get_instance());

	// Run application
	vxg::core::App app(std::move(renderingContext));
	return app.run();
}