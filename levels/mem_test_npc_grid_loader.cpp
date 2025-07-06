#include "mem_test_npc_grid_loader.hpp"

#include "../gen_game_object.hpp"
#include "../coreV/gen_texture.hpp"
#include "../coreV/gen_model.hpp"
#include <memory>
#include <cmath>

namespace gen {

    void MemTestNPCGridLoader::addSurfaceAndGrid(GenDevice& device, GenGameObject::Map& gameObjects) {
        // Surface
        std::shared_ptr<GenModel> surfaceModel = GenModel::createModelFromFile(device, "objectmodels/models/quad.obj");
        auto surface = GenGameObject::createGameObject();
        surface.model = surfaceModel;
        surface.transform.translation = { 0.f, 0.0f, -0.5f };
        surface.transform.scale = glm::vec3(10.f);
        gameObjects.emplace(surface.getId(), std::move(surface));

        // Point light
        auto pointLight = GenGameObject::makePointLight(4.2f, 0.1f);
        pointLight.transform.translation = glm::vec3{ 0.0f, -4.5f, 0.0f };
        pointLight.type = ObjectType::Light;
        gameObjects.emplace(pointLight.getId(), std::move(pointLight));

        // 15x15 Grid of Sound Nodes
        std::shared_ptr<GenModel> nodeModel = GenModel::createModelFromFile(device, "objectmodels/models/sphere.obj");
        //auto texture = std::make_shared<GenTexture>(device, "textures/green.png");

        for (int z = 0; z < 15; ++z) {
            for (int x = 0; x < 15; ++x) {
                glm::vec3 pos = {
                    (x - 7) * 0.6f,
                    -0.5f,
                    (z - 7) * 0.6f
                };

                auto node = GenGameObject::createGameObject();
                node.model = nodeModel;
                //node.texture = texture;
                node.color = glm::vec3{ 1.0f, 0.0f, 0.0f };
                node.soundDisc = std::make_unique<SoundDiscComponent>();
                node.soundDisc->radius = 0.5f;
                node.soundDisc->visible = true;
                node.soundDisc->isPlayerControlled = true;
                node.transform.translation = pos;
                node.transform.scale = glm::vec3(0.1f);
                node.type = ObjectType::Node;
                node.node = std::make_unique<NodeComponent>();
                node.node->selfPosition = pos;

                gameObjects.emplace(node.getId(), std::move(node));
            }
        }
    }

    void MemTestNPCGridLoader::addNPCs(GenDevice& device, GenGameObject::Map& gameObjects, int count) {
        std::shared_ptr<GenModel> npcModel = GenModel::createModelFromFile(device, "objectmodels/models/flat_vase.obj");

        int gridSize = static_cast<int>(std::ceil(std::sqrt(count)));
        int placed = 0;

        for (int z = 0; z < gridSize && placed < count; ++z) {
            for (int x = 0; x < gridSize && placed < count; ++x) {
                glm::vec3 pos = {
                    (x - gridSize / 2) * 1.0f,
                    0.0f,
                    (z - gridSize / 2) * 1.0f
                };

                auto npc = GenGameObject::createGameObject();
                npc.model = npcModel;
                npc.transform.translation = pos;
                npc.transform.scale = glm::vec3(3.f);
                npc.type = ObjectType::NPC;
                npc.npcBehavior = std::make_unique<NPCBehaviorComponent>();
                npc.tag = "memtest_npc";

                gameObjects.emplace(npc.getId(), std::move(npc));
                ++placed;
            }
        }
    }

    void MemTestNPCGridLoader::addNPCs10(GenDevice& device, GenGameObject::Map& gameObjects) {
        addSurfaceAndGrid(device, gameObjects);
        addNPCs(device, gameObjects, 10);
    }

    void MemTestNPCGridLoader::addNPCs50(GenDevice& device, GenGameObject::Map& gameObjects) {
        addSurfaceAndGrid(device, gameObjects);
        addNPCs(device, gameObjects, 50);
    }

    void MemTestNPCGridLoader::addNPCs100(GenDevice& device, GenGameObject::Map& gameObjects) {
        addSurfaceAndGrid(device, gameObjects);
        addNPCs(device, gameObjects, 100);
    }

}
