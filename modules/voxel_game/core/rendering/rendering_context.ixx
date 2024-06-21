module;

#include <memory>
#include <utility>

export module voxel_game.core.rendering:rendering_context;

import :rendering_backend;
import :window_manager;

export namespace vxg::core::rendering {

	template <typename Backend>
	class RenderingContext {
		vxg::core::rendering::RenderingBackend<Backend> m_backend;
		std::unique_ptr<vxg::core::rendering::WindowManager> m_window;

	public:
		RenderingContext(const vxg::core::rendering::WindowProperties& windowProperties)
			: m_backend(Backend {})
		{
			m_window = m_backend.construct_window(windowProperties);
			m_backend.initialize_api();

			m_backend.configure_viewport(0, 0, m_window->resolution().first, m_window->resolution().second);
		}

		// Copy constructor
		RenderingContext(const RenderingContext& rc) = delete;

		// Copy assignment
		RenderingContext& operator=(const RenderingContext& rc) = delete;

		// Move constructor
		RenderingContext(RenderingContext&& rc) noexcept
			: m_backend(std::move(rc.m_backend)), m_window(std::move(rc.m_window)) {}

		// Move assignment
		RenderingContext& operator=(RenderingContext&& rc) noexcept {
			m_backend = std::move(rc.m_backend);
			m_window = std::move(rc.m_window);
		}

		[[nodiscard]]
		vxg::core::rendering::WindowManager& window() noexcept {
			return *m_window;
		}

		[[nodiscard]]
		vxg::core::rendering::RenderingBackend<Backend>& backend() noexcept {
			return m_backend;
		}

		void render_scene() const
			noexcept(noexcept(m_backend.clear_screen()))
		{
			m_backend.clear_screen();

			m_window->swap_buffers();
			m_window->poll_events();
		}
	};

}; // namespace vxg::core::rendering