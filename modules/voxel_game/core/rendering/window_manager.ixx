module;

#include <glad/glad.h>
#include <GLFW/glfw3.h>

#include <utility>
#include <string_view>
#include <string>

export module voxel_game.core.rendering:window_manager;

import voxel_game.exceptions;

export namespace vxg::core::rendering {

	struct APIVersion {
		int8_t major;
		int8_t minor;
	};

	struct WindowProperties {
		std::pair<unsigned int, unsigned int> resolution;
		std::string_view title;
		vxg::core::rendering::APIVersion version;
	};

	class WindowManager {
		GLFWwindow* m_handle;
		std::pair<unsigned int, unsigned int> m_resolution;
		std::string m_title;

	public:
		void make_current() const noexcept {
			glfwMakeContextCurrent(m_handle);
		}

		WindowManager(const WindowProperties& properties)
			: m_handle(nullptr), m_resolution(properties.resolution), m_title(properties.title)
		{
			glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, properties.version.major);
			glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, properties.version.minor);
			glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

			GLFWwindow* window = glfwCreateWindow(properties.resolution.first, properties.resolution.second, m_title.c_str(), nullptr, nullptr);
			if (!window)
				throw vxg::exceptions::InitError("Failed to initialize window.");

			m_handle = window;

			make_current();
		}

		WindowManager() noexcept
			: m_handle(nullptr), m_resolution({}), m_title({}) {}

		~WindowManager() noexcept {
			if (m_handle)
				glfwDestroyWindow(m_handle);
		}

		// Copy constructor
		WindowManager(const WindowManager& wm) = delete;

		// Copy assignment
		WindowManager& operator=(const WindowManager& wm) = delete;

		// Move constructor
		WindowManager(WindowManager&& wm) noexcept {
			m_handle = wm.m_handle;
			m_resolution = std::move(wm.m_resolution);
			m_title = std::move(wm.m_title);
			wm.m_handle = nullptr;
		}

		// Move assignment
		WindowManager& operator=(WindowManager&& wm) noexcept {
			if (this == &wm)
				return *this;
			
			if (wm.m_handle != m_handle && m_handle)
				glfwDestroyWindow(m_handle);

			m_handle = wm.m_handle;
			m_resolution = std::move(wm.m_resolution);
			m_title = std::move(wm.m_title);
			wm.m_handle = nullptr;

			return *this;
		}

		[[nodiscard]]
		GLFWwindow* get() const noexcept {
			return m_handle;
		}

		void swap_buffers() noexcept {
			glfwSwapBuffers(m_handle);
		}

		void poll_events() const noexcept {
			glfwPollEvents();
		}

		[[nodiscard]]
		std::string title() const noexcept {
			return m_title;
		}

		[[nodiscard]]
		std::pair<unsigned int, unsigned int> resolution() const noexcept {
			return m_resolution;
		}

		[[nodiscard]]
		bool should_close() const noexcept {
			return glfwWindowShouldClose(m_handle);
		}
	};

};	// namespace vxg::core