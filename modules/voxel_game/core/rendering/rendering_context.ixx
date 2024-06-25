module;

#include <memory>
#include <utility>
#include <type_traits>
#include <vector>

export module voxel_game.core.rendering:rendering_context;

import :rendering_backend;
import :window_manager;
import :structs;

export namespace vxg::core::rendering {

	struct DrawResourceIdentifier {
		uint16_t memoryPoolIndex;
		uint_fast32_t allocBufferOffset;
		uint_fast32_t drawBufferIndex;
		std::size_t vertexCount;
	};

	template <vxg::core::rendering::rendering_backend_implementation Backend>
	class RenderingContext {
		Backend m_backend;
		std::unique_ptr<vxg::core::rendering::WindowManager> m_window;

	public:
		template<typename... Args>
			requires std::is_constructible_v<Backend, Args&&...>
		RenderingContext(const vxg::core::rendering::WindowProperties& windowProperties, Args&&... backendConstructorArgs)
			: m_backend(Backend {std::forward<Args>(backendConstructorArgs)...})
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
		Backend& backend() noexcept {
			return m_backend;
		}

		// void enqueue_draw_chunk() const noexcept {}

		[[nodiscard]]
		DrawResourceIdentifier enqueue_draw_tri()
			noexcept(std::is_nothrow_invocable_v<decltype(decltype(m_backend)::enqueue_draw), const std::vector<vxg::core::rendering::Vertex>&>)
		{
			std::vector<vxg::core::rendering::Vertex> verts{
				{.position = {-0.5, -0.5, 0.0}},
				{.position = {0.0, 0.5, 0.0}},
				{.position = {0.5, -0.5, 0.0}}
			};
			
			auto alloc = m_backend.copy_to_vram(verts);
			auto drawCommand = m_backend.enqueue_draw(alloc);

			return DrawResourceIdentifier{
				.memoryPoolIndex = alloc.memoryPoolIndex,
				.allocBufferOffset = alloc.bufferOffset,
				.drawBufferIndex = drawCommand.bufferIndex,
				.vertexCount = alloc.vertexCount
			};
		}

		void dequeue_draw(const DrawResourceIdentifier& drawResource) {
			m_backend.dequeue_draw(vxg::core::rendering::DrawCommandIdentifier{
				.memoryPoolIndex = drawResource.memoryPoolIndex,
				.bufferIndex = drawResource.drawBufferIndex
			});

			m_backend.deallocate_vram(vxg::core::rendering::GPUAllocationIdentifier{
				.memoryPoolIndex = drawResource.memoryPoolIndex,
				.bufferOffset = drawResource.allocBufferOffset,
				.vertexCount = drawResource.vertexCount
			});
		}

		void draw_queued() const
			noexcept(noexcept(m_backend.draw_queued()))
		{
			m_backend.draw_queued();
		}

		void draw_and_present() const
			noexcept(noexcept(m_backend.clear_screen()) && noexcept(draw_queued()))
		{
			m_backend.clear_screen();

			draw_queued();

			m_window->swap_buffers();
			m_window->poll_events();
		}
	};

}; // namespace vxg::core::rendering