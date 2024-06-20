module;

#include <memory>

#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>

export module voxel_game.core.rendering:rendering_backend;

import :window_manager;
import voxel_game.exceptions;

export namespace vxg::core::rendering {

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
		void terminate()
			noexcept(noexcept(derived_instance()->terminate_impl()))
		{
			m_terminated = true;
			derived_instance()->terminate_impl();
		}
		
		virtual ~RenderingBackend()
			// noexcept(noexcept(terminate()))  -- This causes a compilation error???
		{
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
		RenderingBackend& operator=(RenderingBackend&& rb) = delete;

		void initialize()
			noexcept(noexcept(derived_instance()->initialize_impl()))
		{
			derived_instance()->initialize_impl();
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

		void draw_scene() const
			noexcept(noexcept(clear_screen()))
		{
			clear_screen();
		}
	};

	class OpenGLBackend final : public RenderingBackend<OpenGLBackend> {
		using Base = RenderingBackend<OpenGLBackend>;
		friend Base;

		// Depends on glfw being initialized
		void initialize_impl() {
			if (!gladLoadGLLoader(reinterpret_cast<GLADloadproc>(glfwGetProcAddress)))
				throw vxg::exceptions::LoadError("Failed to load OpenGL symbols.");
		}

		void terminate_impl() noexcept {
			glfwTerminate();
		}
		
		std::unique_ptr<WindowManager> construct_window_impl(const vxg::core::rendering::WindowProperties& properties) const {
			return std::make_unique<vxg::core::rendering::WindowManager>(properties);
		}

		void clear_screen_impl() const noexcept {
			glClear(GL_COLOR_BUFFER_BIT);
		}

		void set_clear_color_impl(const glm::vec4& color) noexcept {
			glClearColor(color.r, color.g, color.b, color.a);
		}
	
	public:
		OpenGLBackend() : Base() {}
	};

}; // namespace vxg::core::rendering