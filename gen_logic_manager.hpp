#pragma once

#include "gen_game_object.hpp"

namespace gen {

    class GenLogicManager {
    public:
        GenLogicManager() = default;
        ~GenLogicManager() = default;

        GenLogicManager(const GenLogicManager&) = delete;
        GenLogicManager& operator=(const GenLogicManager&) = delete;

        void update(float dt, GenGameObject::Map& gameObjects);
    };

}