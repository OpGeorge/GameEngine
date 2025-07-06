#pragma once

#include "../coreV/gen_device.hpp"
#include "../gen_game_object.hpp"

namespace gen {
    class MemTestNPCGridLoader {
    public:
        static void addNPCs10(GenDevice& device, GenGameObject::Map& gameObjects);
        static void addNPCs50(GenDevice& device, GenGameObject::Map& gameObjects);
        static void addNPCs100(GenDevice& device, GenGameObject::Map& gameObjects);

    private:
        static void addSurfaceAndGrid(GenDevice& device, GenGameObject::Map& gameObjects);
        static void addNPCs(GenDevice& device, GenGameObject::Map& gameObjects, int count);
    };
}
