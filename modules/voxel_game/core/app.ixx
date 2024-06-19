module;

#include <glad/glad.h>
#include <GLFW/glfw3.h>

#include <utility>
#include <string_view>

export module voxel_game.core:app;

import :window_manager;
import voxel_game.exceptions;
import voxel_game.utilities;
import voxel_game.typedefs;

export namespace vxg::core {

	class App {
		void load_opengl() {
			if (!gladLoadGLLoader(reinterpret_cast<GLADloadproc>(glfwGetProcAddress)))
				throw vxg::exceptions::LoadError("Failed to load OpenGL symbols.");
		}

		void set_opengl_rendering_state() noexcept {
			glClearColor(0.0, 0.0, 0.0, 1.0);
		}

		void do_render_loop(const WindowManager& window) noexcept {
			while (!glfwWindowShouldClose(window.get())) {
				glClear(GL_COLOR_BUFFER_BIT);

				glfwSwapBuffers(window.get());
				glfwPollEvents();
			}
		}

	public:
		vxg::ExitCode run(const WindowManager& window) noexcept {
			// TODO: Add resizing callback

			try { load_opengl(); }
			catch (const vxg::exceptions::LoadError& e)
				{ return vxg::exceptions::handle_unrecoverable_error(e); }

			glViewport(0, 0, window.resolution().first, window.resolution().second);

			set_opengl_rendering_state();
			do_render_loop(window);

			return EXIT_SUCCESS;
		}

	};	// namespace vxg::core
}