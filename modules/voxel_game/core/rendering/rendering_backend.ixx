module;

#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>

#include <memory>
#include <vector>
#include <type_traits>
#include <cstdint>

export module voxel_game.core.rendering:rendering_backend;

import :window_manager;
import :structs;
import voxel_game.exceptions;

export namespace vxg::core::rendering {

	struct GPUAllocationIdentifier {
		uint16_t memoryPoolIndex;
		uint_fast32_t bufferOffset;
		std::size_t vertexCount;
	};

	struct DrawCommandIdentifier {
		uint16_t memoryPoolIndex;
		uint_fast32_t bufferIndex;
	};

	template <typename Derived>
	class RenderingBackend {
		bool m_terminated = false;
		
		RenderingBackend() {
			if (!glfwInit())
				throw vxg::exceptions::InitError("Failed to initialise GLFW.");
		}
		friend Derived;


		const Derived* derived_instance() const noexcept {
			return static_cast<const Derived*>(this);
		}

		Derived* derived_instance() noexcept {
			return static_cast<Derived*>(this);
		}

	public:
		void terminate() noexcept {
			m_terminated = true;
			derived_instance()->terminate_impl();
		}
		
		virtual ~RenderingBackend() noexcept {
			if (!m_terminated)
				terminate();
		}

		// Copy constructor
		RenderingBackend(const RenderingBackend& rb) = delete;

		// Copy assignment
		RenderingBackend& operator=(const RenderingBackend& rb) = delete;

		// Move constructor
		RenderingBackend(RenderingBackend&& rb) noexcept {
			rb.m_terminated = true;
		}

		// Move assignment
		RenderingBackend& operator=(RenderingBackend&& rb) noexcept {
			rb.m_terminated = true;
		}

		void initialize_api()
			noexcept(noexcept(derived_instance()->initialize_api_impl()))
		{
			derived_instance()->initialize_api_impl();
		}

		std::unique_ptr<WindowManager> construct_window(const vxg::core::rendering::WindowProperties& properties) const
			noexcept(noexcept(derived_instance()->construct_window_impl(properties)))
		{
			return derived_instance()->construct_window_impl(properties);
		}

		void clear_screen() const
			noexcept(noexcept(derived_instance()->clear_screen_impl()))
		{
			derived_instance()->clear_screen_impl();
		}

		void set_clear_color(const glm::vec4& color)
			noexcept(noexcept(derived_instance()->set_clear_color_impl(color)))
		{
			derived_instance()->set_clear_color_impl(color);
		}

		[[nodiscard]]
		GPUAllocationIdentifier copy_to_vram(const std::vector<vxg::core::rendering::Vertex>& verts)
			noexcept(noexcept(derived_instance()->copy_to_vram_impl(verts)))
		{
			return derived_instance()->copy_to_vram_impl(verts);
		}

		void deallocate_vram(const GPUAllocationIdentifier& alloc)
			noexcept(noexcept(derived_instance()->deallocate_vram_impl(alloc)))
		{
			derived_instance()->deallocate_vram_impl(alloc);
		}

		[[nodiscard]]
		DrawCommandIdentifier enqueue_draw(const GPUAllocationIdentifier& alloc)
			noexcept(noexcept(derived_instance()->enqueue_draw_impl(alloc)))
		{
			return derived_instance()->enqueue_draw_impl(alloc);
		}

		void dequeue_draw(const DrawCommandIdentifier& drawCommand)
			noexcept(noexcept(derived_instance()->dequeue_draw_impl(drawCommand)))
		{
			derived_instance()->dequeue_draw_impl(drawCommand);
		}

		void draw_queued() const
			noexcept(noexcept(derived_instance()->draw_queued_impl()))
		{
			derived_instance()->draw_queued_impl();
		}

		void configure_viewport(unsigned int x, unsigned int y, unsigned int width, unsigned int height)
			noexcept(noexcept(derived_instance()->configure_viewport_impl(x, y, width, height)))
		{
			derived_instance()->configure_viewport_impl(x, y, width, height);
		}
	};

	template <typename T>
	concept rendering_backend_implementation = std::is_base_of_v<RenderingBackend<T>, T>;

}; // namespace vxg::core::rendering