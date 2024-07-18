module;

#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include <memory>
#include <utility>
#include <type_traits>
#include <vector>

export module voxel_game.core.rendering:rendering_context;

import :rendering_backend;
import :window_manager;
import :camera;
import voxel_game.core.structs;
import voxel_game.core.memory;
import voxel_game.core.rendering.meshing;

export namespace vxg::core::rendering {

	template <vxg::core::rendering::rendering_backend_implementation Backend, vxg::core::rendering::meshing::mesher_implementation Mesher>
	class RenderingContext {
		Backend m_backend;
		Mesher m_mesher;
		std::unique_ptr<vxg::core::rendering::WindowManager> m_window;

		double m_lastTime = 0.0;
		double m_deltaTime = 0.0;

	public:
		using AllocationIdentifier = Backend::AllocatorType::ObjectAllocationIdentifier;

		template<typename CtorMesher, typename... Args>
			requires std::is_constructible_v<Backend, Args&&...> && std::is_same_v<Mesher, CtorMesher>
		RenderingContext(CtorMesher&& mesher, const vxg::core::rendering::WindowProperties& windowProperties, Args&&... backendConstructorArgs)
			: m_backend(Backend{std::forward<Args>(backendConstructorArgs)...}), m_mesher(std::forward<CtorMesher>(mesher))
		{
			m_window = m_backend.construct_window(windowProperties);
			m_backend.initialize_api();

			m_backend.configure_viewport(0, 0, m_window->resolution().width, m_window->resolution().height);

			m_lastTime = glfwGetTime();
		}

		// Copy constructor
		RenderingContext(const RenderingContext& rc) = delete;

		// Copy assignment
		RenderingContext& operator=(const RenderingContext& rc) = delete;

		// Move constructor
		RenderingContext(RenderingContext&& rc) noexcept
			: m_backend(std::move(rc.m_backend)), m_mesher(std::move(rc.m_mesher)), m_window(std::move(rc.m_window)) {}

		// Move assignment
		RenderingContext& operator=(RenderingContext&& rc) noexcept {
			m_backend = std::move(rc.m_backend);
			m_mesher = std::move(rc.m_mesher);
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

		[[nodiscard]]
		Mesher& mesher() noexcept {
			return m_mesher;
		}

		[[nodiscard]]
		AllocationIdentifier enqueue_draw_tri(glm::vec3 position)
			noexcept(std::is_nothrow_invocable_v<decltype(&Backend::construct_object), Backend, const std::vector<vxg::core::structs::Vertex>&, vxg::core::structs::ObjectData> &&
				std::is_nothrow_invocable_v<decltype(&Backend::enqueue_draw), Backend, AllocationIdentifier>)
		{
			std::vector<vxg::core::structs::Vertex> verts{
				{.position = {0.0, 0.5, 0.0}},
				{.position = {-0.5, -0.5, 0.0}},
				{.position = {0.5, -0.5, 0.0}}
			};

			auto modelMatrix = glm::translate(glm::mat4(1.0f), position);
			
			vxg::core::structs::ObjectData data{
				.modelMatrix = modelMatrix
			};
			
			auto alloc = m_backend.construct_object(verts, data);
			m_backend.enqueue_draw(alloc);

			return alloc;
		}

		[[nodiscard]]
		AllocationIdentifier enqueue_draw_chunk(const std::vector<bool>& voxels, glm::vec3 position)
			noexcept(noexcept(m_mesher.mesh(voxels)) &&
				std::is_nothrow_invocable_v<decltype(&Backend::construct_object), Backend, const std::vector<vxg::core::structs::Vertex>&, vxg::core::structs::ObjectData> &&
				std::is_nothrow_invocable_v<decltype(&Backend::enqueue_draw), Backend, AllocationIdentifier>)
		{
			auto verts = m_mesher.mesh(voxels);

			auto modelMatrix = glm::translate(glm::mat4(1.0f), position);
			vxg::core::structs::ObjectData data{
				.modelMatrix = modelMatrix
			};

			auto alloc = m_backend.construct_object(verts, data);
			m_backend.enqueue_draw(alloc);

			return alloc;
		}

		void dequeue_draw(const AllocationIdentifier& drawResource) {
			m_backend.dequeue_draw(drawResource);
			m_backend.destroy_object(drawResource);
		}

		void draw_queued(const Camera& camera)
			noexcept(noexcept(m_backend.draw_queued()) &&
				std::is_nothrow_invocable_v<decltype(&Backend::update_uniforms), Backend, typename Backend::UniformType>)
		{
			m_backend.update_uniforms({
				.viewMatrix = camera.view_matrix(),
				.projectionMatrix = camera.projection_matrix()
			});
			m_backend.draw_queued();
		}

		void draw_and_present(const Camera& camera)
			noexcept(noexcept(m_backend.clear_screen()) && noexcept(draw_queued(camera)))
		{
			m_backend.clear_screen();

			draw_queued(camera);

			m_window->swap_buffers();
			m_window->poll_events();
		}

		void update_delta_time() noexcept {
			auto currentTime = glfwGetTime();
			m_deltaTime = currentTime - m_lastTime;
			m_lastTime = currentTime;
		}

		[[nodiscard]]
		double delta_time() const noexcept {
			return m_deltaTime;
		}
	};

}; // namespace vxg::core::rendering