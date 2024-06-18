module;

#include <glad/glad.h>
#include <GLFW/glfw3.h>

#include <utility>
#include <string_view>

export module voxel_game.core:app;

import voxel_game.exceptions;
import voxel_game.utilities;
import voxel_game.typedefs;

export namespace vxg::core {

	class App {
	public:
		struct WindowProperties {
			std::pair<int, int> resolution;
			std::string_view title;
		};

	private:
		GLFWwindow* create_window(const WindowProperties& properties) {
			glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
			glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 6);
			glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

			GLFWwindow* window = glfwCreateWindow(properties.resolution.first, properties.resolution.second, properties.title.data(), nullptr, nullptr);
			if (!window)
				throw vxg::exceptions::InitError("Failed to initialize window.");

			glfwMakeContextCurrent(window);

			return window;
		}

		void load_opengl() {
			if (!gladLoadGLLoader(reinterpret_cast<GLADloadproc>(glfwGetProcAddress)))
				throw vxg::exceptions::LoadError("Failed to load OpenGL symbols.");
		}

		void init_glfw() {
			if (!glfwInit())
				throw vxg::exceptions::InitError("Failed to initialise GLFW.");
		}

		void set_opengl_rendering_state() noexcept {
			glClearColor(0.0, 0.0, 0.0, 1.0);
		}

		void do_render_loop(GLFWwindow* window) noexcept {
			while (!glfwWindowShouldClose(window)) {
				glClear(GL_COLOR_BUFFER_BIT);

				glfwSwapBuffers(window);
				glfwPollEvents();
			}
		}

	public:
		vxg::ExitCode run(const WindowProperties& windowProperties) noexcept {
			try { init_glfw(); }
			catch (const vxg::exceptions::InitError& e)
				{ return vxg::exceptions::handle_unrecoverable_error(e); }
			vxg::utilities::DeferredFunction deferredGLFWTerminate(glfwTerminate);

			GLFWwindow* window;
			try { window = create_window(windowProperties); }
			catch (const vxg::exceptions::LoadError& e)
				{ return vxg::exceptions::handle_unrecoverable_error(e); }

			// TODO: Add resizing callback

			try { load_opengl(); }
			catch (const vxg::exceptions::LoadError& e)
				{ return vxg::exceptions::handle_unrecoverable_error(e); }

			glViewport(0, 0, windowProperties.resolution.first, windowProperties.resolution.second);


			set_opengl_rendering_state();
			do_render_loop(window);

			return EXIT_SUCCESS;
		}

	};	// namespace vxg::core
}