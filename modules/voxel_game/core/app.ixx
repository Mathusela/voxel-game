module;

#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>

#include <utility>
#include <string_view>

export module voxel_game.core:app;

import voxel_game.core.rendering;
import voxel_game.exceptions;
import voxel_game.utilities;
import voxel_game.typedefs;

export namespace vxg::core {

	template <typename Backend>
	class App {
		vxg::core::rendering::RenderingBackend<Backend> m_backend;

		void set_rendering_state() noexcept {
			m_backend.set_clear_color(glm::vec4(0.0, 0.0, 0.0, 1.0));
		}

		void do_render_loop(const vxg::core::rendering::WindowManager& window) noexcept {
			while (!glfwWindowShouldClose(window.get())) {
				m_backend.clear_screen();

				glfwSwapBuffers(window.get());
				glfwPollEvents();
			}
		}

	public:
		App(vxg::core::rendering::RenderingBackend<Backend>&& backend) noexcept
			: m_backend(std::move(backend)) {}

		vxg::ExitCode run(const vxg::core::rendering::WindowManager& window) noexcept {
			// TODO: Add resizing callback

			try { m_backend.initialize(); }
			catch (const vxg::exceptions::LoadError& e)
				{ return vxg::exceptions::handle_unrecoverable_error(e); }

			m_backend.configure_viewport(0, 0, window.resolution().first, window.resolution().second);

			set_rendering_state();
			do_render_loop(window);

			return EXIT_SUCCESS;
		}

	};	// namespace vxg::core
}