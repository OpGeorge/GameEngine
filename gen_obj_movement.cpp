#include "gen_obj_movement.hpp"
#include <glm/glm.hpp>
#include <glm/gtc/constants.hpp>
#include <limits>
#include <cmath>

namespace gen {

    void GameObjectMovementController::moveInPlaneXZ(GLFWwindow* window, float dt, GenGameObject& object) {
        glm::vec3 rotate{ 0 };
        if (glfwGetKey(window, keys.lookRight) == GLFW_PRESS) rotate.y += 1.f;
        if (glfwGetKey(window, keys.lookLeft) == GLFW_PRESS) rotate.y -= 1.f;
        if (glfwGetKey(window, keys.lookUp) == GLFW_PRESS) rotate.x += 1.f;
        if (glfwGetKey(window, keys.lookDown) == GLFW_PRESS) rotate.x -= 1.f;

        if (glm::dot(rotate, rotate) > std::numeric_limits<float>::epsilon()) {
            object.transform.rotation += lookSpeed * dt * glm::normalize(rotate);
        }

        object.transform.rotation.x = glm::clamp(object.transform.rotation.x, -1.5f, 1.5f);
        object.transform.rotation.y = glm::mod(object.transform.rotation.y, glm::two_pi<float>());

        float yaw = object.transform.rotation.y;
        const glm::vec3 forwardDir(sin(yaw), 0.f, cos(yaw));
        const glm::vec3 rightDir(forwardDir.z, 0.f, -forwardDir.x);
        const glm::vec3 upDir(0.f, -2.50f, 0.f);

        glm::vec3 moveDir{ 0.f };
        if (glfwGetKey(window, keys.moveForward) == GLFW_PRESS) moveDir += forwardDir;
        if (glfwGetKey(window, keys.moveBackward) == GLFW_PRESS) moveDir -= forwardDir;
        if (glfwGetKey(window, keys.moveRight) == GLFW_PRESS) moveDir += rightDir;
        if (glfwGetKey(window, keys.moveLeft) == GLFW_PRESS) moveDir -= rightDir;
        if (glfwGetKey(window, keys.moveUp) == GLFW_PRESS) moveDir += upDir;
        if (glfwGetKey(window, keys.moveDown) == GLFW_PRESS) moveDir -= upDir;

        if (glm::dot(moveDir, moveDir) > std::numeric_limits<float>::epsilon()) {
            object.transform.translation += moveSpeed * dt * glm::normalize(moveDir);
        }

        // Optional clamp to floor
        if (object.transform.translation.y > 0.f) {
            object.transform.translation.y = 0.f;
        }
    }

    void GameObjectMovementController::updateRotationFromMouse(GLFWwindow* window, GenGameObject& object) {
        // Optional: implement if you want mouse-based object rotation
        // (can reuse logic from camera controller)
    }

}
