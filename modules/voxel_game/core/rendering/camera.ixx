module;

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/quaternion.hpp>

#include <glad/glad.h>
#include <GLFW/glfw3.h>

export module voxel_game.core.rendering:camera;

import :window_manager;
import voxel_game.core.rendering.structs;

using namespace vxg::core::rendering::structs;

export namespace vxg::core::rendering {

	class Camera {
	protected:
		glm::mat4 m_viewMatrix{};
		glm::mat4 m_projectionMatrix{};

	public:
		glm::vec3 position;
		glm::quat rotation;

		Camera() noexcept = default;
		Camera(glm::vec3 position, glm::quat rotation) noexcept
			: position(position), rotation(rotation) {}

		Camera(const Camera&) = default;
		Camera& operator=(const Camera&) = default;
		virtual ~Camera() noexcept = default;

		virtual void update() noexcept = 0;
		
		[[nodiscard]]
		const glm::mat4& view_matrix() const noexcept {
			return m_viewMatrix;
		};
		
		[[nodiscard]]
		const glm::mat4& projection_matrix() const noexcept {
			return m_projectionMatrix;
		};
	};

	class PerspectiveCamera final : public Camera {
	public:
		float fov;
		float aspectRatio;
		float nearClip;
		float farClip;

		PerspectiveCamera(glm::vec3 position, glm::quat rotation, float aspectRatio, float fov, float nearClip, float farClip) noexcept
			: Camera(position, rotation), fov(fov), aspectRatio(aspectRatio), nearClip(nearClip), farClip(farClip)
		{
			update();
		}
		PerspectiveCamera(glm::vec3 position, glm::quat rotation, ScreenSize screenSize, float fov, float nearClip, float farClip) noexcept
			: PerspectiveCamera(position, rotation, static_cast<float>(screenSize.width)/static_cast<float>(screenSize.height), fov, nearClip, farClip) {}
		PerspectiveCamera(float aspectRatio, float fov, float nearClip, float farClip) noexcept 
			: PerspectiveCamera(glm::vec3(0.0f), glm::quat(glm::vec3(0.0f, 0.0f, 0.0f)), aspectRatio, fov, nearClip, farClip) {}
		PerspectiveCamera(ScreenSize screenSize, float fov, float nearClip, float farClip) noexcept
			: PerspectiveCamera(glm::vec3(0.0f), glm::quat(glm::vec3(0.0f, 0.0f, 0.0f)), screenSize, fov, nearClip, farClip) {}

		void update_screen_size(const ScreenSize& screenSize) noexcept {
			aspectRatio = static_cast<float>(screenSize.width) / static_cast<float>(screenSize.height);
		}

		void update() noexcept override {
			m_projectionMatrix = glm::perspective(fov, aspectRatio, nearClip, farClip);
			m_viewMatrix = glm::translate(glm::mat4_cast(rotation), -position);
		}
	};

}; // namespace vxg::core::rendering