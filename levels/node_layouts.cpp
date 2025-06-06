#include "node_layouts.hpp"
#include "../coreV/gen_model.hpp"

namespace gen {

    void addNodeCluster1(GenDevice& device, GenGameObject::Map& gameObjects, float xOffset, float zOffset) {
        std::shared_ptr<GenModel> nodeModel = GenModel::createModelFromFile(device, "objectmodels/models/sphere.obj");

        std::vector<glm::vec3> positions;

        // 1. Two rows in front of player (Z = 0.5 and -0.5)
        for (int row = 0; row < 2; ++row) {
            float z = 0.5f - row * 1.0f;
            for (int i = -5; i <= 5; ++i) {
                float x = static_cast<float>(i * 1.2f);
                positions.emplace_back(x + xOffset, -0.5f, z + zOffset);
            }
        }

        // 2. Hallway: 5 nodes down the center (X = 0), Z = -1.5 to -5.5
        for (int i = 0; i < 5; ++i) {
            float z = -1.5f - i * 1.0f;
            positions.emplace_back(0.0f + xOffset, -0.5f, z + zOffset);
        }

        // 3. Four more rows (Z = -6.5 to -9.5)
        for (int row = 0; row < 4; ++row) {
            float z = -6.5f - row * 1.0f;
            for (int i = -5; i <= 5; ++i) {
                float x = static_cast<float>(i * 1.2f);
                positions.emplace_back(x + xOffset, -0.5f, z + zOffset);
            }
        }

        for (const auto& pos : positions) {
            auto node = GenGameObject::createGameObject();
            node.model = nodeModel;
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

    void addNodeCluster2(GenDevice& device, GenGameObject::Map& gameObjects, float xOffset, float zOffset) {
        std::shared_ptr<GenModel> nodeModel = GenModel::createModelFromFile(device, "objectmodels/models/sphere.obj");

        std::vector<glm::vec3> positions;

        // 1. Two rows in front of player (Z = 0.5 and -0.5)
        for (int row = 0; row < 2; ++row) {
            float z = 0.5f - row * 1.0f;
            for (int i = -5; i <= 5; ++i) {
                float x = static_cast<float>(i * 1.2f);
                positions.emplace_back(x + xOffset, -0.5f, z + zOffset);
            }
        }

        // Left hallway (X = -6.0)
        for (int i = 0; i < 5; ++i) {
            float z = -1.5f - i * 1.0f;
            positions.emplace_back(-6.0f + xOffset, -0.5f, z + zOffset);
        }

        // Right hallway (X = +6.0)
        for (int i = 0; i < 5; ++i) {
            float z = -1.5f - i * 1.0f;
            positions.emplace_back(6.0f + xOffset, -0.5f, z + zOffset);
        }

        // 3. Four more rows (Z = -6.5 to -9.5)
        for (int row = 0; row < 4; ++row) {
            float z = -6.5f - row * 1.0f;
            for (int i = -5; i <= 5; ++i) {
                float x = static_cast<float>(i * 1.2f);
                positions.emplace_back(x + xOffset, -0.5f, z + zOffset);
            }
        }

        for (const auto& pos : positions) {
            auto node = GenGameObject::createGameObject();
            node.model = nodeModel;
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
