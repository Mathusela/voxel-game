module;

#include <glm/glm.hpp>
#include <glm/gtc/quaternion.hpp>

#include <glad/glad.h>
#include <GLFW/glfw3.h>

#include <iostream>

export module voxel_game.core.logic.camera:camera_controller;

import voxel_game.core.rendering;

export namespace vxg::core::logic::camera {

	// TODO: FINISH THIS
	void camera_controller(vxg::core::rendering::Camera& camera, const vxg::core::rendering::WindowManager& window, double deltaTime) noexcept {
		float cameraMovementSpeed = 3.0f * static_cast<float>(deltaTime);
		float cameraRotationSpeed = 90.0f * static_cast<float>(deltaTime);

		if (glfwGetKey(window.get(), GLFW_KEY_LEFT_CONTROL))
			cameraMovementSpeed *= 10.0f;

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
		
		glm::vec3 forward = glm::normalize(glm::vec3(0.0, 0.0, 1.0) * camera.rotation);
		glm::vec3 right = glm::cross(forward, glm::vec3(0.0, 1.0, 0.0));
		if (glfwGetKey(window.get(), GLFW_KEY_UP))
			camera.rotation *= glm::angleAxis(glm::radians(cameraRotationSpeed), right);
		if (glfwGetKey(window.get(), GLFW_KEY_DOWN))
			camera.rotation *= glm::angleAxis(glm::radians(-cameraRotationSpeed), right);

		if (glm::length(offset) != 0)
			offset = glm::normalize(offset);

		forward.y = 0.0f;
		right.y = 0.0f;

		glm::mat3 flatRotationMatrix = glm::mat3( glm::lookAt(camera.position, camera.position - forward, glm::vec3(0.0, 1.0, 0.0)) );
		glm::quat flatRotation = glm::quat_cast(flatRotationMatrix);

		camera.position += cameraMovementSpeed * offset * flatRotation;

		camera.update();
	}

}; // namespace vxg::core::logic::camera