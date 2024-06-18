module;

#include <glad/glad.h>
#include <GLFW/glfw3.h>

#include <utility>
#include <string_view>
#include <iostream>

export module voxel_game.core:app;

import voxel_game.exceptions;
import voxel_game.utilities;

export namespace vxg::core {

	// TODO: Go over all of this
	class App {
		struct WindowProperties {
			std::pair<int, int> resolution;
			std::string_view title;
		};

		GLFWwindow* create_window(const WindowProperties& properties) {
			glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
			glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 6);
			glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

			GLFWwindow* window = glfwCreateWindow(properties.resolution.first, properties.resolution.second, properties.title.data(), nullptr, nullptr);

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

		void set_opengl_rendering_state() {
			glClearColor(0.0, 0.0, 0.0, 1.0);
		}

		void render_loop(GLFWwindow* window) {
			while (!glfwWindowShouldClose(window)) {
				glClear(GL_COLOR_BUFFER_BIT);

				glfwSwapBuffers(window);
				glfwPollEvents();
			}
		}

	public:
		int run() {
			try { init_glfw(); }
			catch (const vxg::exceptions::InitError& e) {
				std::cerr << e.what();
				return EXIT_FAILURE;
			}
			vxg::utilities::DeferredFunction deferredGLFWTerminate(glfwTerminate);

			constexpr WindowProperties windowProperties{ {700, 500}, "Voxel Game" };
			auto window = create_window(windowProperties);

			// TODO: Add resizing callback

			try { load_opengl(); }
			catch (const vxg::exceptions::LoadError& e) {
				std::cerr << e.what();
				return EXIT_FAILURE;
			}

			glViewport(0, 0, windowProperties.resolution.first, windowProperties.resolution.second);


			set_opengl_rendering_state();
			render_loop(window);

			return EXIT_SUCCESS;
		}

	};	// namespace vxg::core
}