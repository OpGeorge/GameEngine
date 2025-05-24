#include "gen_logic_manager.hpp"
#include <iostream>
#include <set>

namespace gen {

    void GenLogicManager::update(float dt, GenGameObject::Map& gameObjects) {
        static std::set<std::pair<GenGameObject::id_t, GenGameObject::id_t>> loggedPairs;

        for (auto& [id, obj] : gameObjects) {

            if (obj.type != ObjectType::Camera &&
                obj.type != ObjectType::Light &&
                obj.type != ObjectType::Sphere &&
                obj.transform.translation.y < 0.f) {
                obj.transform.translation.y += 0.3f * dt; // gravity effect
            }

            if (obj.transform.translation.y > 0.f) {
                obj.transform.translation.y = 0.f;
            }

            if (!obj.soundDisc) continue;

            glm::vec2 center = { obj.transform.translation.x, obj.transform.translation.z };
            float radius = obj.soundDisc->radius;

            for (auto& [otherId, target] : gameObjects) {
                if (id == otherId) continue;
                if (!target.soundDisc) continue;

                glm::vec2 targetPos = { target.transform.translation.x, target.transform.translation.z };
                float dist = glm::length(center - targetPos);
                float combinedRadius = radius + target.soundDisc->radius;

                if (dist <= combinedRadius) {
                    // Normalize pair so (id, otherId) == (otherId, id)
                    auto pair = std::make_pair(std::min(id, otherId), std::max(id, otherId));
                    if (loggedPairs.insert(pair).second) {
                        std::cout << "Disc overlap: " << pair.first << " <-> " << pair.second << "\n";
                    }

                    if (target.soundDisc->visible) {
                        target.color = glm::vec3(0.0f, 1.0f, 0.0f);
                    }
                }
            }
        }
    }

}