#include "keyboard_movement_controller.hpp"
#include <iostream>

namespace gen {


	void KeyboardMovementController::moveInPlaneXZ(GLFWwindow* window, float dt, GenGameObject& gameObject) {


		glm::vec3 rotate{ 0 };
		if (glfwGetKey(window, keys.lookRight) == GLFW_PRESS) {

			rotate.y += 1.f;

		}
		if (glfwGetKey(window, keys.lookLeft) == GLFW_PRESS) {

			rotate.y -= 1.f;

		}
		if (glfwGetKey(window, keys.lookUp) == GLFW_PRESS) {

			rotate.x += 1.f;

		}
		if (glfwGetKey(window, keys.lookDown) == GLFW_PRESS) {

			rotate.x -= 1.f;

		}

		if (glm::dot(rotate, rotate) > std::numeric_limits<float>::epsilon()) {

			gameObject.transform.rotation += lookSpeed * dt * glm::normalize(rotate);

		}


		gameObject.transform.rotation.x = glm::clamp(gameObject.transform.rotation.x, -1.5f, 1.5f);
		gameObject.transform.rotation.y = glm::mod(gameObject.transform.rotation.y, glm::two_pi<float>());

		float yaw = gameObject.transform.rotation.y;
		const glm::vec3 forwardDir(sin(yaw), 0.f, cos(yaw));
		const glm::vec3 rightDir(forwardDir.z, 0.f, -forwardDir.x);
		const glm::vec3 upDir(0.f, -1.f, 0.f);

		glm::vec3 moveDir{ 0.f };

		if (glfwGetKey(window, keys.moveForward) == GLFW_PRESS) {
		
			moveDir += forwardDir;

		}

		if (glfwGetKey(window, keys.moveBackward) == GLFW_PRESS) {

			moveDir -= forwardDir;

		}
		if (glfwGetKey(window, keys.moveRight) == GLFW_PRESS) {

			moveDir += rightDir;

		}
		if (glfwGetKey(window, keys.moveLeft) == GLFW_PRESS) {

			moveDir -= rightDir;

		}
		if (glfwGetKey(window, keys.moveUp) == GLFW_PRESS) {

			moveDir += upDir;

		}
		if (glfwGetKey(window, keys.moveDown) == GLFW_PRESS) {

			moveDir -= upDir;

		}

		if (glm::dot(moveDir, moveDir) > std::numeric_limits<float>::epsilon()) {

			gameObject.transform.translation += moveSpeed * dt * glm::normalize(moveDir);

		}

		if (gameObject.transform.translation.y > 0.f) {
			gameObject.transform.translation.y = 0.f;
		}

		//std::cout << gameObject.transform.translation.x << "  X  " << std::endl;
		//std::cout << gameObject.transform.translation.z << "  Z  " << std::endl;
	}


	void KeyboardMovementController::processMouseLookToggle(GLFWwindow* window) {
		bool isTogglePressed = glfwGetKey(window, GLFW_KEY_V) == GLFW_PRESS;
		if (isTogglePressed && !lastToggleState) {
			mouseLookEnabled = !mouseLookEnabled;

			if (mouseLookEnabled) {
				glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
				glfwGetCursorPos(window, &lastMouseX, &lastMouseY);  // reset reference point
			}
			else {
				glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
			}
		}
		lastToggleState = isTogglePressed;
	}

	void KeyboardMovementController::updateCameraViewFromMouse(GLFWwindow* window, GenGameObject& cameraObject) {
		if (!mouseLookEnabled) return;

		double mouseX, mouseY;
		glfwGetCursorPos(window, &mouseX, &mouseY);

		//std::cout << "Pos X Y : " << mouseX << " " << mouseY << "\n";

		double deltaX = mouseX - lastMouseX;
		double deltaY = mouseY - lastMouseY;
		lastMouseX = mouseX;
		lastMouseY = mouseY;

		cameraObject.transform.rotation.y += static_cast<float>(deltaX) * mouseSensitivity;
		cameraObject.transform.rotation.x -= static_cast<float>(deltaY) * mouseSensitivity;

		
		cameraObject.transform.rotation.x = glm::clamp(cameraObject.transform.rotation.x, -1.5f, 1.5f);
	}

}