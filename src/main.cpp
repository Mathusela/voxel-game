#include <utility>
#include <exception>
#include <cstdint>
#include <string_view>

#include <iostream>

import voxel_game.core;
import voxel_game.exceptions;
import voxel_game.utilities;

// NOTE: Module local definitions are still namespaced due to MSVC tooling limitations leading to linker errors

// === TODO (high priority) ===
// TODO: Documentation
// TODO: Update README
// TODO: Comprehensive error handling
// TODO: Comprehensive logging
// TODO: Add resize callback
// TODO: Finish camera_controller function
// TODO: Shader loading / embedding
// TODO: Add chunk boundary awareness / merging in all meshing logic
// TODO: Optimize greedy mesher
// TODO: Voxel materials
// TODO: Bindless textures - load all textures into VRAM for whole program lifetime (bindless avoids texture binding point limits)
// TODO: Deferred rendering

// === TODO (medium priority) ===
// TODO: Move all the code using structs to using reflection
// TODO: Find a way to handle different types in the allocation initialization code (e.g. Mat4)
// TODO: Frustrum culling

// === TODO (low priority) ===
// TODO: Rename raw AllocationIdentifier struct in opengl_allocator to OpenGLAllocationIdentifier
// TODO: Define NDEBUG for release builds
// TODO: If doing GPU meshing and allocation without returning to CPU minimum free size (max size of chunk)
// TODO: Error handling for program linking
// TODO: Ensure that construction (especially data construct) in OpenGLBackend works if input struct is padded
// TODO: Add option to update portion of UBO (e.g. may only have to update matrices)
// TODO: Do a review of the entire codebase checking for style consistency
// TODO: Do a review of the entire codebase checking for const correctness

// === FIXME / known issues ===
// FIXME: Doesnt draw all meshes on Intel iGPU (Test on other iGPUs?)
// TODO: Make sure destructors don't throw if exceptions are thrown in initialization

// === Logging ===
// TODO: Add log levels/filtering
// TODO: Make logging thread-safe
// TODO: Flush log file during program runtime
// TODO: Add buffer to logging
// TODO: Multi-threaded logging

// === Reflection ===
// TODO: Documentation and error handling/asserts in reflection code
// TODO: Can I add support for templated classes?
// TODO: Enum support
// TODO: Universal class printer

int main() {
	// Construct rendering context
	using Allocator = vxg::core::memory::OpenGLAllocator;
	using Backend = vxg::core::rendering::OpenGLBackend<Allocator>;
	using Mesher = vxg::core::rendering::meshing::GreedyMesher;
	using Context = vxg::core::rendering::RenderingContext<Backend, Mesher>;

	auto contextConstructionResult = vxg::exceptions::construct_and_catch<Context, std::exception>(
		vxg::exceptions::handle_unrecoverable_error<std::exception>,
		Mesher({ 16, 16, 16 }),
		vxg::core::rendering::WindowProperties{ {1400, 900}, "Voxel Game", {4, 6}, 4 },
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