#include "gen_logic_manager.hpp"
#include <iostream>
#include <set>
#include <cmath>


namespace gen {

    void GenLogicManager::update(float dt, GenGameObject::Map& gameObjects) {
        static std::set<std::pair<GenGameObject::id_t, GenGameObject::id_t>> loggedPairs;

        static float timeAccum = 0.f;
        timeAccum += dt;

        for (auto& [id, obj] : gameObjects) {

            if (obj.type != ObjectType::Camera &&
                obj.type != ObjectType::Light &&
                obj.type != ObjectType::Sphere &&
                obj.type != ObjectType::Goal &&
                obj.type != ObjectType::Wall &&
                obj.type != ObjectType::Node &&
                obj.type != ObjectType::FinalGoal&&
                obj.transform.translation.y < 0.f) {
                obj.transform.translation.y += 0.7f * dt; // gravity effect
            }

            if (obj.transform.translation.y > 0.f) {
                obj.transform.translation.y = 0.f;
            }

            if (obj.type == ObjectType::Goal) {
                // Oscillate Y between 2.0 and 3.0
                float amplitude = 0.5f;
                float offset = -1.f;
                float oscillation = std::sin(timeAccum * 2.0f) * amplitude + offset;
                obj.transform.translation.y = oscillation;

                // Clean in-place rotation around Y-axis
                obj.transform.rotation.y += glm::radians(45.0f) * dt;
            }

            if (obj.type == ObjectType::FinalGoal) {

                obj.transform.rotation.y += glm::radians(45.0f) * dt;
            
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
                        //std::cout << "Disc overlap: " << pair.first << " <-> " << pair.second << "\n";
                    }

                    if (target.soundDisc->visible) {
                        target.color = glm::vec3(0.0f, 1.0f, 0.0f);
                    }
                }
            }
        }
    }

}