module;

#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>

#include <memory>
#include <vector>
#include <type_traits>

export module voxel_game.core.rendering:rendering_backend;

import :window_manager;
import :camera;
import voxel_game.core.structs;
import voxel_game.core.memory;
import voxel_game.exceptions;
import voxel_game.utilities.tmp;

export namespace vxg::core::rendering {

	template <typename T>
	struct RenderingBackendTraits;

	template <typename Derived>
	class RenderingBackend {
		bool m_terminated = false;
		bool m_initialized = false;
		
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
		using ObjectAllocationIdentifier = vxg::utilities::tmp::ExtractNestedTemplate_t<Derived>::ObjectAllocationIdentifier;
		using UniformType = typename RenderingBackendTraits<Derived>::UniformType;

		void terminate() noexcept {
			derived_instance()->terminate_impl();
			m_terminated = true;
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
			m_initialized = rb.m_initialized;
			m_terminated = rb.m_terminated;
			rb.m_terminated = true;
		}

		// Move assignment
		RenderingBackend& operator=(RenderingBackend&& rb) noexcept {
			m_initialized = rb.m_initialized;
			m_terminated = rb.m_terminated;
			rb.m_terminated = true;
		}

		void initialize_api()
			noexcept(noexcept(derived_instance()->initialize_api_impl()))
		{
			derived_instance()->initialize_api_impl();
			m_initialized = true;
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
		ObjectAllocationIdentifier construct_object(const std::vector<vxg::core::structs::Vertex>& verts, const vxg::core::structs::ObjectData& data)
			noexcept(noexcept(derived_instance()->construct_object_impl(verts, data)))
		{
			return derived_instance()->construct_object_impl(verts, data);
		}

		void destroy_object(const ObjectAllocationIdentifier& object)
			noexcept(noexcept(derived_instance()->destroy_object_impl(object)))
		{
			return derived_instance()->destroy_object_impl(object);
		}

		void enqueue_draw(const ObjectAllocationIdentifier& object)
			noexcept(noexcept(derived_instance()->enqueue_draw_impl(object)))
		{
			derived_instance()->enqueue_draw_impl(object);
		}

		void dequeue_draw(const ObjectAllocationIdentifier& object)
			noexcept(noexcept(derived_instance()->dequeue_draw_impl(object)))
		{
			derived_instance()->dequeue_draw_impl(object);
		}

		void draw_queued() const
			noexcept(noexcept(derived_instance()->draw_queued_impl()))
		{
			derived_instance()->draw_queued_impl();
		}

		void update_uniforms(const UniformType& data)
			noexcept(noexcept(derived_instance()->update_uniforms_impl(data)))
		{
			derived_instance()->update_uniforms_impl(data);
		}

		void configure_viewport(unsigned int x, unsigned int y, unsigned int width, unsigned int height)
			noexcept(noexcept(derived_instance()->configure_viewport_impl(x, y, width, height)))
		{
			derived_instance()->configure_viewport_impl(x, y, width, height);
		}

		void enable_depth_testing()
			noexcept(noexcept(derived_instance()->enable_depth_testing_impl()))
		{
			derived_instance()->enable_depth_testing_impl();
		}

		void enable_face_culling()
			noexcept(noexcept(derived_instance()->enable_face_culling_impl()))
		{
			derived_instance()->enable_face_culling_impl();
		}

		void enable_multisampling()
			noexcept(noexcept(derived_instance()->enable_multisampling_impl()))
		{
			derived_instance()->enable_multisampling_impl();
		}

		void enable_wireframe()
			noexcept(noexcept(derived_instance()->enable_wireframe_impl()))
		{
			derived_instance()->enable_wireframe_impl();
		}

		void disable_wireframe()
			noexcept(noexcept(derived_instance()->disable_wireframe_impl()))
		{
			derived_instance()->disable_wireframe_impl();
		}
	};

	template <typename T>
	concept rendering_backend_implementation = std::is_base_of_v<RenderingBackend<T>, T>;

}; // namespace vxg::core::rendering