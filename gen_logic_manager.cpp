#include "gen_logic_manager.hpp"

namespace gen {

    void GenLogicManager::update(float dt, GenGameObject::Map& gameObjects) {
        for (auto& [id, obj] : gameObjects) {

            if (obj.type != ObjectType::Camera && obj.type != ObjectType::Light && obj.transform.translation.y < 0.f) {
                obj.transform.translation.y += 0.3f * dt;
                
            }
            
            if (obj.transform.translation.y > 0.f) {
                obj.transform.translation.y = 0.f;
            }

            
        }
    }

}