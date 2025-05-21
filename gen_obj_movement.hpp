#pragma once

#include "gen_game_object.hpp"
#include "coreV/gen_window.hpp"

namespace gen {

    class GameObjectMovementController {
    public:
        struct KeyMappings {
            int moveLeft = GLFW_KEY_J;
            int moveRight = GLFW_KEY_L;
            int moveForward = GLFW_KEY_I;
            int moveBackward = GLFW_KEY_K;
            int moveUp = GLFW_KEY_U;
            int moveDown = GLFW_KEY_O;
            int lookLeft = GLFW_KEY_LEFT;
            int lookRight = GLFW_KEY_RIGHT;
            int lookUp = GLFW_KEY_UP;
            int lookDown = GLFW_KEY_DOWN;
        };

        KeyMappings keys{};
        float moveSpeed{ 3.0f };
        float lookSpeed{ 1.5f };

        void moveInPlaneXZ(GLFWwindow* window, float dt, GenGameObject& object);
        void updateRotationFromMouse(GLFWwindow* window, GenGameObject& object);
    };

}