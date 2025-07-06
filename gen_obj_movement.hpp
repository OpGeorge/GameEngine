#pragma once

#include "gen_game_object.hpp"
#include "coreV/gen_window.hpp"

namespace gen {

    class GameObjectMovementController {
    public:
        struct KeyMappings {
            int moveLeft = GLFW_KEY_A;
            int moveRight = GLFW_KEY_D;
            int moveForward = GLFW_KEY_W;
            int moveBackward = GLFW_KEY_S;
            int moveUp = GLFW_KEY_Q;
            int moveDown = GLFW_KEY_Z;
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