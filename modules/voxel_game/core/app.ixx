module;

#include <glfw/glfw3.h>
#include <glm/glm.hpp>

#include <utility>
#include <cstdlib>

export module voxel_game.core:app;

import voxel_game.core.rendering;
import voxel_game.exceptions;
import voxel_game.utilities;
import voxel_game.typedefs;
import voxel_game.core.logic.camera;

export namespace vxg::core {

	template <typename Backend>
	class App {
		vxg::core::rendering::RenderingContext<Backend> m_renderingContext;

		void set_rendering_state() noexcept {
			m_renderingContext.backend().set_clear_color(glm::vec4(0.0, 0.0, 0.0, 1.0));
			//m_renderingContext.backend().enable_depth_testing();
			m_renderingContext.backend().enable_multisampling();
		}

		void render_loop() noexcept {
			auto resolution = m_renderingContext.window().resolution();
			vxg::core::rendering::PerspectiveCamera camera(resolution, 45.0f, 0.1f, 200.0f);
			camera.position = glm::vec3(0, 0.0f, 1.0f);
			camera.update();

			auto lastTime = glfwGetTime();
			while (!m_renderingContext.window().should_close()) {
				auto currentTime = glfwGetTime();
				auto deltaTime = currentTime - lastTime;
				lastTime = currentTime;
				vxg::core::logic::camera::camera_controller(camera, m_renderingContext.window(), deltaTime);
				m_renderingContext.draw_and_present(camera);
			}
		}

	public:
		App(vxg::core::rendering::RenderingContext<Backend>&& renderingContext) noexcept
			: m_renderingContext(std::move(renderingContext)) {}

		vxg::ExitCode run() noexcept {
			// TODO: Add resizing callback
			set_rendering_state();

			//[[maybe_unused]] auto test = m_renderingContext.enqueue_draw_tri({0.0, 1.0, 0.0});
			for (int i=0; i<1'000; i++)
				[[maybe_unused]] auto temp = m_renderingContext.enqueue_draw_tri({i%100, (i/100)%100, -i/10'000});

			render_loop();

			return EXIT_SUCCESS;
		}

	};	// namespace vxg::core
}