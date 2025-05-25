#pragma once

#include "../coreV/gen_device.hpp"
#include "../gen_game_object.hpp"

#include <string>

namespace gen {

    class Level1Loader {
    public:
        // You pass in the Vulkan device so models/textures can be created.
        static GenGameObject::Map loadLevel1(GenDevice& device);
    };

}