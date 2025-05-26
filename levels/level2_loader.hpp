#pragma once

#include "../coreV/gen_device.hpp"
#include "../gen_game_object.hpp"

#include <string>

namespace gen {

    class Level2Loader {
    public:
        // You pass in the Vulkan device so models/textures can be created.
        static void loadLevel2(GenDevice& device, GenGameObject::Map& gameObjects);
        static glm::vec3 getStartingCameraPos();
    private:
        static glm::vec3 startingCameraPos;

    };

}