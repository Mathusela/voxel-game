module;

#include <glad/glad.h>
#include <GLFW/glfw3.h>

#include <cstdint>
#include <utility>
#include <string_view>
#include <string>

export module voxel_game.core:window_manager;

import voxel_game.exceptions;

export namespace vxg::core {

	struct APIVersion {
		int8_t major;
		int8_t minor;
	};

	struct WindowManager {
	private:
		GLFWwindow* m_handle;
		std::pair<int, int> m_resolution;
		std::string m_title;

	public:
		
		void make_current() const noexcept {
			glfwMakeContextCurrent(m_handle);
		}

		WindowManager(std::pair<int, int> resolution, const std::string_view title, [[maybe_unused]] APIVersion version)
			: m_handle(nullptr), m_resolution(resolution), m_title(title)
		{
			glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, version.major);
			glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, version.minor);
			glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

			GLFWwindow* window = glfwCreateWindow(resolution.first, resolution.second, m_title.c_str(), nullptr, nullptr);
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

		[[nodiscard]]
		std::string title() const noexcept {
			return m_title;
		}

		[[nodiscard]]
		std::pair<int, int> resolution() const noexcept {
			return m_resolution;
		}
	};

};	// namespace vxg::core