#pragma once
#include "../coreV/gen_device.hpp"
#include "../gen_game_object.hpp"

#include <string>

namespace gen {
    class MemTestLoader {
    public:
        static void addNodes100(GenDevice& device, GenGameObject::Map& gameObjects);
        static void addNodes500(GenDevice& device, GenGameObject::Map& gameObjects);
        static void addNodes1000(GenDevice& device, GenGameObject::Map& gameObjects);

    private:
        static void addSurface(GenDevice& device, GenGameObject::Map& gameObjects);
        static void addSphericalNodes(GenDevice& device, GenGameObject::Map& gameObjects, int count);
    };
}