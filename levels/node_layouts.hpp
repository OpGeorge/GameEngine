#pragma once

#include "../coreV/gen_device.hpp"
#include "../gen_game_object.hpp"

namespace gen {
    void addNodeCluster1(GenDevice& device, GenGameObject::Map& gameObjects, float xOffset = 0.0f, float zOffset = 0.0f);
    void addNodeCluster2(GenDevice& device, GenGameObject::Map& gameObjects, float xOffset = 0.0f, float zOffset = 0.0f);
    void addNodeCluster3(GenDevice& device, GenGameObject::Map& gameObjects);
}