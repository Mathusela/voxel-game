module;

#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>

#include <utility>

export module voxel_game.core:app;

import voxel_game.core.rendering;
import voxel_game.exceptions;
import voxel_game.utilities;
import voxel_game.typedefs;

export namespace vxg::core {

	template <typename Backend>
	class App {
		vxg::core::rendering::RenderingContext<Backend> m_renderingContext;

		void set_rendering_state() noexcept {
			m_renderingContext.backend().set_clear_color(glm::vec4(0.0, 0.0, 0.0, 1.0));
		}

		void do_render_loop() noexcept {
			while (!m_renderingContext.window().should_close()) {
				m_renderingContext.render_scene();
			}
		}

	public:
		App(vxg::core::rendering::RenderingContext<Backend>&& renderingContext) noexcept
			: m_renderingContext(std::move(renderingContext)) {}

		vxg::ExitCode run() noexcept {
			// TODO: Add resizing callback
			set_rendering_state();
			do_render_loop();

			return EXIT_SUCCESS;
		}

	};	// namespace vxg::core
}