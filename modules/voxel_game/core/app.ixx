module;

#include <glm/glm.hpp>

#include <utility>
#include <iostream>

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

		void render_loop() noexcept {
			while (!m_renderingContext.window().should_close()) {
				m_renderingContext.draw_and_present();
			}
		}

	public:
		App(vxg::core::rendering::RenderingContext<Backend>&& renderingContext) noexcept
			: m_renderingContext(std::move(renderingContext)) {}

		vxg::ExitCode run() noexcept {
			// TODO: Add resizing callback
			set_rendering_state();

			[[maybe_unused]] auto test = m_renderingContext.enqueue_draw_tri({0.0, 1.0, 0.0});
			for (int i = 0; i < 900; i++)
				[[maybe_unused]] auto temp = m_renderingContext.enqueue_draw_tri({i, 0.0, 0.0});

			render_loop();

			return EXIT_SUCCESS;
		}

	};	// namespace vxg::core
}