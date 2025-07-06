#include "mem_test_loader.hpp"


#include "../gen_game_object.hpp"
#include "../coreV/gen_texture.hpp"
#include "../coreV/gen_model.hpp"
#include "node_layouts.hpp"
#include <memory>

namespace gen {

    void MemTestLoader::addSurface(GenDevice& genDevice, GenGameObject::Map& gameObjects) {
        std::shared_ptr<GenModel> genModel = GenModel::createModelFromFile(genDevice, "objectmodels/models/quad.obj");

        auto surface = GenGameObject::createGameObject();
        surface.model = genModel;
        surface.transform.translation = { 0.f,0.0f,-0.5f };
        surface.transform.scale = glm::vec3(10.f);
        gameObjects.emplace(surface.getId(), std::move(surface));

        auto pointLight = GenGameObject::makePointLight(4.2f, 0.1f);
        pointLight.transform.translation = glm::vec3{ 0.0f,-4.5f,-0.0f };
        pointLight.type = ObjectType::Light;
        gameObjects.emplace(pointLight.getId(), std::move(pointLight));
    }

    void MemTestLoader::addSphericalNodes(GenDevice& device, GenGameObject::Map& gameObjects, int count) {
        std::shared_ptr<GenModel> nodeModel = GenModel::createModelFromFile(device, "objectmodels/models/sphere.obj");
        auto texture = std::make_shared<GenTexture>(device, "textures/green.png");
        int gridSize = static_cast<int>(std::ceil(std::sqrt(count)));
        int added = 0;

        for (int z = 0; z < gridSize && added < count; ++z) {
            for (int x = 0; x < gridSize && added < count; ++x) {
                glm::vec3 pos = {
                    (x - gridSize / 2) * 0.6f,
                    -0.5f,
                    (z - gridSize / 2) * 0.6f
                };

                auto node = GenGameObject::createGameObject();
                node.model = nodeModel;
                node.texture= texture;
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
                ++added;
            }
        }
    }

    void MemTestLoader::addNodes100(GenDevice& device, GenGameObject::Map& gameObjects) {
        addSurface(device, gameObjects);
        addSphericalNodes(device, gameObjects, 100);
    }

    void MemTestLoader::addNodes500(GenDevice& device, GenGameObject::Map& gameObjects) {
        addSurface(device, gameObjects);
        addSphericalNodes(device, gameObjects, 500);
    }

    void MemTestLoader::addNodes1000(GenDevice& device, GenGameObject::Map& gameObjects) {
        addSurface(device, gameObjects);
        addSphericalNodes(device, gameObjects, 1000);
    }

}
