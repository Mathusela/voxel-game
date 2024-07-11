#include <utility>
#include <exception>
#include <cstdint>
#include <string_view>

#include <iostream>

import voxel_game.core;
import voxel_game.exceptions;
import voxel_game.utilities;

// NOTE: Module local definitions are still namespaced due to MSVC tooling limitations leading to linker errors

// TODO: If doing GPU meshing and allocation without returning to CPU minimum free size (max size of chunk)
// TODO: Bindless textures - load all textures into VRAM for whole program lifetime (bindless avoids texture binding point limits)
// TODO: Define NDEBUG for release builds
// TODO: Rename raw AllocationIdentifier struct in opengl_allocator to OpenGLAllocationIdentifier
// TODO: Add log levels/filtering
// TODO: Make logging thread-safe
// TODO: Flush log file during program runtime
// TODO: Add buffer to logging
// TODO: Ensure that construction (especially data construct) in OpenGLBackend works if input struct is padded
// TODO: Add more comprehensive error handling and logging
// TODO: Make sure destructors don't throw if exceptions are thrown in initialization
// TODO: Could I multi-thread allocation?

// TODO: Move all the code using structs to using reflection
// TODO: Find a way to handle different types in the allocation initialization code (e.g. Mat4)
// TODO: Documentation and error handling/asserts in reflection code
// TODO: Can I add support for templated classes?
// TODO: Enum support

// TODO: Move camera stuff to update uniforms function (using UBO), could use set_camera in the backend?
// TODO: Use ScreenSize (currently in camera.ixx) in WindowManager
// TODO: Move camera to its own namespace within rendering
// TODO: Finish camera_controller function

// FIXME: Is OpenGLAllocator::initialize() running 3 times?

int main() {
	// Construct rendering context
	using Allocator = vxg::core::memory::OpenGLAllocator;
	using Backend = vxg::core::rendering::OpenGLBackend<Allocator>;
	using Context = vxg::core::rendering::RenderingContext<Backend>;

	auto contextConstructionResult = vxg::exceptions::construct_and_catch<Context, std::exception>(
		vxg::exceptions::handle_unrecoverable_error<std::exception>,
		vxg::core::rendering::WindowProperties{ {1000, 700}, "Voxel Game", {4, 6}, 4 },
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