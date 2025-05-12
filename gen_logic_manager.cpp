#include "gen_logic_manager.hpp" 
#include <iostream>

namespace gen {

    void GenLogicManager::update(float dt, GenGameObject::Map& gameObjects) {
        for (auto& [id, obj] : gameObjects) {

            if (obj.type != ObjectType::Camera &&
                obj.type != ObjectType::Light &&
                obj.type != ObjectType::Sphere &&
                obj.transform.translation.y < 0.f) {
               
                obj.transform.translation.y += 0.3f * dt;
                
            }
            
            if (obj.transform.translation.y > 0.f) {
                obj.transform.translation.y = 0.f;
            }

            if (!obj.soundDisc) continue;  // make sure the source has a disc

            glm::vec2 center = { obj.transform.translation.x, obj.transform.translation.z };
            float radius = obj.soundDisc->radius;

            for (auto& [otherId, target] : gameObjects) {
                if (id == otherId) continue;
                if (!target.soundDisc) continue;  // critical check

                glm::vec2 targetPos = { target.transform.translation.x, target.transform.translation.z };
                float dist = glm::length(center - targetPos);
                float combinedRadius = radius + target.soundDisc->radius;

                if (dist <= combinedRadius) {
                    std::cout << "Disc overlap: " << id << " <-> " << otherId << "\n";

                    // Now safe to use target.soundDisc
                    if (target.soundDisc->visible) {
                        target.color = glm::vec3(0.0f, 1.0f, 0.0f);
                    }
                }
            }
            
        }
    }

}