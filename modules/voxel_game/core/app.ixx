module;

#include <glad/glad.h>
#include <glfw/glfw3.h>
#include <glm/glm.hpp>

#include <utility>
#include <cstdlib>

#include <vector>
#include <iostream>

export module voxel_game.core:app;

import voxel_game.core.rendering;
import voxel_game.exceptions;
import voxel_game.utilities;
import voxel_game.typedefs;
import voxel_game.core.logic.camera;

export namespace vxg::core {

	template <typename Context>
	class App {
		Context m_renderingContext;

		void set_rendering_state() noexcept {
			m_renderingContext.backend().set_clear_color(glm::vec4(0.0, 0.0, 0.0, 1.0));
			m_renderingContext.backend().enable_depth_testing();
			m_renderingContext.backend().enable_multisampling();
			// Cannot use face culling with greedy mesher since some faces may need to be viewed from both sides
			// m_renderingContext.backend().enable_face_culling();
		}

		void render_loop(std::vector<bool> voxels, [[maybe_unused]] decltype(m_renderingContext)::AllocationIdentifier chunk) noexcept {
			auto resolution = m_renderingContext.window().resolution();
			// TODO: Change far plane back to something reasonable
			vxg::core::rendering::PerspectiveCamera camera(resolution, 45.0f, 0.1f, 2000.0f);
			camera.position = glm::vec3(0, 0.0f, 1.0f);
			camera.update();

			while (!m_renderingContext.window().should_close()) {
				m_renderingContext.update_delta_time();
				vxg::core::logic::camera::camera_controller(camera, m_renderingContext.window(), m_renderingContext.delta_time());

				if (glfwGetKey(m_renderingContext.window().get(), GLFW_KEY_F) == GLFW_PRESS) {
					static bool wireframe = false;
					if (!wireframe) m_renderingContext.backend().enable_wireframe();
					else m_renderingContext.backend().disable_wireframe();
					wireframe = !wireframe;
				}
				
				if (glfwGetKey(m_renderingContext.window().get(), GLFW_KEY_Q) == GLFW_PRESS) {
					static long editing = static_cast<long>(voxels.size()-1);
					m_renderingContext.dequeue_draw(chunk);
					voxels[editing] = !voxels[editing];
					editing--;
					if (editing == -1)
						editing = static_cast<long>(voxels.size()-1);
					chunk = m_renderingContext.enqueue_draw_chunk(voxels, { 0.0, 0.0, 0.0 });
				}
				
				m_renderingContext.draw_and_present(camera);
			}
		}

	public:
		App(Context&& renderingContext) noexcept
			: m_renderingContext(std::move(renderingContext)) {}

		vxg::ExitCode run() noexcept {
			// TODO: Add resizing callback
			set_rendering_state();

			glm::vec3 chunkSize = m_renderingContext.mesher().chunkSize;

			std::vector<bool> voxels(static_cast<size_t>(chunkSize.x*chunkSize.y*chunkSize.z));
			for (size_t i = 0; i < voxels.size(); i++) {
				//voxels[i] = i%static_cast<int>(chunkSize.x) <= (i/static_cast<int>(chunkSize.y))%static_cast<int>(chunkSize.x);
				voxels[i] = true;
			}

			size_t numVerts = 0;
			auto initialChunk = m_renderingContext.enqueue_draw_chunk(voxels, {0.0, 0.0, 0.0});
			numVerts += initialChunk.vertices.size;
			
			for (int i = 1; i < 1000; i++) {
				auto test = m_renderingContext.enqueue_draw_chunk(voxels, {(i%150)*chunkSize.x, (i/(150*150))*chunkSize.y*2, ((i/150)%150)*chunkSize.z});
				numVerts += test.vertices.size;
			}

			render_loop(voxels, initialChunk);
			std::cout << numVerts << "\n";

			return EXIT_SUCCESS;
		}

	};	// namespace vxg::core
}