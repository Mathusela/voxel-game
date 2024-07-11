module;

#include <glm/glm.hpp>
#include <glm/gtc/quaternion.hpp>

#include <glad/glad.h>
#include <GLFW/glfw3.h>


export module voxel_game.core.logic.camera:camera_controller;

import voxel_game.core.rendering;

export namespace vxg::core::logic::camera {

	// TODO: FINISH THIS
	void camera_controller(vxg::core::rendering::Camera& camera, const vxg::core::rendering::WindowManager& window, double deltaTime) noexcept {
		float cameraMovementSpeed = 3.0f * static_cast<float>(deltaTime);
		float cameraRotationSpeed = 90.0f * static_cast<float>(deltaTime);

		auto offset = glm::vec3(0.0);
		if (glfwGetKey(window.get(), GLFW_KEY_W))
			offset += glm::vec3(0.0, 0.0, -1.0);
		if (glfwGetKey(window.get(), GLFW_KEY_S))
			offset += glm::vec3(0.0, 0.0, 1.0);
		if (glfwGetKey(window.get(), GLFW_KEY_A))
			offset += glm::vec3(-1.0, 0.0, 0.0);
		if (glfwGetKey(window.get(), GLFW_KEY_D))
			offset += glm::vec3(1.0, 0.0, 0.0);
		if (glfwGetKey(window.get(), GLFW_KEY_SPACE))
			offset += glm::vec3(0.0, 1.0, 0.0);
		if (glfwGetKey(window.get(), GLFW_KEY_LEFT_SHIFT))
			offset += glm::vec3(0.0, -1.0, 0.0);

		if (glfwGetKey(window.get(), GLFW_KEY_LEFT))
			camera.rotation *= glm::angleAxis(glm::radians(-cameraRotationSpeed), glm::vec3(0.0, 1.0, 0.0));
		if (glfwGetKey(window.get(), GLFW_KEY_RIGHT))
			camera.rotation *= glm::angleAxis(glm::radians(cameraRotationSpeed), glm::vec3(0.0, 1.0, 0.0));

		if (glm::length(offset) != 0)
			offset = glm::normalize(offset);

		camera.position += cameraMovementSpeed * offset * camera.rotation;

		camera.update();
	}

}; // namespace vxg::core::logic::camera