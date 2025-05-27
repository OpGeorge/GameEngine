#include "level1_loader.hpp"


#include "../gen_game_object.hpp"
#include "../coreV/gen_texture.hpp"
#include "../coreV/gen_model.hpp"
#include <memory>

namespace gen {


    glm::vec3 Level1Loader::startingCameraPos{};
    
    glm::vec3 Level1Loader::getStartingCameraPos() {
        return startingCameraPos;
    }

    void Level1Loader::loadLevel1(GenDevice& genDevice, GenGameObject::Map& gameObjects) {
        
        startingCameraPos = glm::vec3(0.f, 0.0f, 1.f);

        auto texture = std::make_shared<GenTexture>(genDevice, "textures/vaseTexture.png");

        std::shared_ptr<GenModel> genModel = GenModel::createModelFromFile(genDevice, "objectmodels/models/smooth_vase.obj");
        auto vase = GenGameObject::createGameObject();
        vase.model = genModel;
        vase.texture = texture;
        vase.type = ObjectType::Player;
        vase.transform.translation = { -1.0f, -2.0f, 2.5f };
        vase.transform.scale = glm::vec3(3.f);
        vase.soundDisc = std::make_unique<SoundDiscComponent>();
        vase.soundDisc->radius = 1.0f;
        vase.soundDisc->visible = true;
        vase.soundDisc->isPlayerControlled = true;

        gameObjects.emplace(vase.getId(), std::move(vase));

        // Repeat for other objects like flat vase, cube, lights, etc.
        genModel = GenModel::createModelFromFile(genDevice, "objectmodels/models/flat_vase.obj");
        auto flatVase = GenGameObject::createGameObject();
        flatVase.model = genModel;
        flatVase.transform.translation = { 1.f,0.0f,0.f };
        flatVase.transform.scale = glm::vec3(3.f);
        flatVase.type = ObjectType::NPC;
        gameObjects.emplace(flatVase.getId(), std::move(flatVase)); // make sure the move has a valid pointer and a not null obj

        genModel = GenModel::createModelFromFile(genDevice, "objectmodels/models/colored_cube.obj");
        auto coloredCube = GenGameObject::createGameObject();
        coloredCube.model = genModel;
        coloredCube.transform.translation = { 0.f,-.5f,0.f };
        coloredCube.transform.scale = glm::vec3(0.5f);
        gameObjects.emplace(coloredCube.getId(), std::move(coloredCube));

        //ground
        genModel = GenModel::createModelFromFile(genDevice, "objectmodels/models/quad.obj");

        auto surface = GenGameObject::createGameObject();
        surface.model = genModel;
        surface.transform.translation = { 0.f,0.0f,0.f };
        surface.transform.scale = glm::vec3(3.f);
        gameObjects.emplace(surface.getId(), std::move(surface));

        // first sound node
        genModel = GenModel::createModelFromFile(genDevice, "objectmodels/models/sphere.obj");
        auto sphere = GenGameObject::createGameObject();
        sphere.model = genModel;
        sphere.color = glm::vec3{ 1.0f, 0.0f, 0.0f };
        sphere.soundDisc = std::make_unique<SoundDiscComponent>();
        sphere.soundDisc->radius = .5f;
        sphere.soundDisc->visible = true;
        sphere.soundDisc->isPlayerControlled = true;
        sphere.transform.translation = { -1.f,-0.5f,-1.5f };
        sphere.transform.scale = glm::vec3(.1f);
        sphere.type = ObjectType::Node;
        sphere.node = std::make_unique<NodeComponent>();
        sphere.node->selfPosition = sphere.transform.translation;
        gameObjects.emplace(sphere.getId(), std::move(sphere));

        genModel = GenModel::createModelFromFile(genDevice, "objectmodels/models/sphere.obj");
        auto sphere2 = GenGameObject::createGameObject();
        sphere2.model = genModel;
        sphere2.color = glm::vec3{ 1.0f, 0.0f, 0.0f };
        sphere2.soundDisc = std::make_unique<SoundDiscComponent>();
        sphere2.soundDisc->radius = .5f;
        sphere2.soundDisc->visible = true;
        sphere2.soundDisc->isPlayerControlled = true;
        sphere2.transform.translation = { -3.f,-0.5f,-1.5f };
        sphere2.transform.scale = glm::vec3(.1f);
        sphere2.type = ObjectType::Node;
        sphere2.node = std::make_unique<NodeComponent>();
        sphere2.node->selfPosition = sphere2.transform.translation;

        gameObjects.emplace(sphere2.getId(), std::move(sphere2));

        genModel = GenModel::createModelFromFile(genDevice, "objectmodels/models/sphere.obj");
        auto sphere3 = GenGameObject::createGameObject();
        sphere3.model = genModel;
        sphere3.color = glm::vec3{ 1.0f, 0.0f, 0.0f };
        sphere3.soundDisc = std::make_unique<SoundDiscComponent>();
        sphere3.soundDisc->radius = .5f;
        sphere3.soundDisc->visible = true;
        sphere3.soundDisc->isPlayerControlled = true;
        sphere3.transform.translation = { 1.f,-0.5f,-1.5f };
        sphere3.transform.scale = glm::vec3(.1f);
        sphere3.type = ObjectType::Node;
        sphere3.node = std::make_unique<NodeComponent>();
        sphere3.node->selfPosition = sphere3.transform.translation;

        gameObjects.emplace(sphere3.getId(), std::move(sphere3));

        genModel = GenModel::createModelFromFile(genDevice, "objectmodels/models/sphere.obj");
        auto sphere4 = GenGameObject::createGameObject();
        sphere4.model = genModel;
        sphere4.color = glm::vec3{ 1.0f, 0.0f, 0.0f };
        sphere4.soundDisc = std::make_unique<SoundDiscComponent>();
        sphere4.soundDisc->radius = .5f;
        sphere4.soundDisc->visible = true;
        sphere4.soundDisc->isPlayerControlled = true;
        sphere4.transform.translation = { -1.f,-0.5f,.5f };
        sphere4.transform.scale = glm::vec3(.1f);
        sphere4.type = ObjectType::Node;
        sphere4.node = std::make_unique<NodeComponent>();
        sphere4.node->selfPosition = sphere4.transform.translation;
        gameObjects.emplace(sphere4.getId(), std::move(sphere4));

        genModel = GenModel::createModelFromFile(genDevice, "objectmodels/models/sphere.obj");
        auto sphere5 = GenGameObject::createGameObject();
        sphere5.model = genModel;
        sphere5.color = glm::vec3{ 1.0f, 0.0f, 0.0f };
        sphere5.soundDisc = std::make_unique<SoundDiscComponent>();
        sphere5.soundDisc->radius = .5f;
        sphere5.soundDisc->visible = true;
        sphere5.soundDisc->isPlayerControlled = true;
        sphere5.transform.translation = { -3.f,-0.5f,.5f };
        sphere5.transform.scale = glm::vec3(.1f);
        sphere5.type = ObjectType::Node;
        sphere5.node = std::make_unique<NodeComponent>();
        sphere5.node->selfPosition = sphere5.transform.translation;

        gameObjects.emplace(sphere5.getId(), std::move(sphere5));


        //last sound node
        genModel = GenModel::createModelFromFile(genDevice, "objectmodels/models/sphere.obj");
        auto sphere6 = GenGameObject::createGameObject();
        sphere6.model = genModel;
        sphere6.color = glm::vec3{ 1.0f, 0.0f, 0.0f };
        sphere6.soundDisc = std::make_unique<SoundDiscComponent>();
        sphere6.soundDisc->radius = .5f;
        sphere6.soundDisc->visible = true;
        sphere6.soundDisc->isPlayerControlled = true;
        sphere6.transform.translation = { 1.f,-0.5f,.5f };
        sphere6.transform.scale = glm::vec3(.1f);
        sphere6.type = ObjectType::Node;
        sphere6.node = std::make_unique<NodeComponent>();
        sphere6.node->selfPosition = sphere6.transform.translation;

        gameObjects.emplace(sphere6.getId(), std::move(sphere6));



        { // lumina alba din cub
            auto pointLight = GenGameObject::makePointLight(0.2f);
            pointLight.transform.translation = glm::vec3{ .0f,-.5f,.0f };
            gameObjects.emplace(pointLight.getId(), std::move(pointLight));
        }


        //point lights declaration
        std::vector<glm::vec3> lightColors{
            {1.f, .1f, .1f},
            {.1f, .1f, 1.f},
            {.1f, 1.f, .1f},
            {1.f, 1.f, .1f},
            {.1f, 1.f, 1.f},
            {1.f, 1.f, 1.f}  //
        };

        for (int i = 0; i < lightColors.size(); i++) {
            auto pointLight = GenGameObject::makePointLight(0.2f);
            pointLight.type = ObjectType::Light;
            pointLight.color = lightColors[i];
            auto rotateLight = glm::rotate(glm::mat4(1.f), (i * glm::two_pi<float>()) / lightColors.size(), { 0.f,-1.f,0.f });
            pointLight.transform.translation = glm::vec3(rotateLight * glm::vec4(-1.f, -1.f, -1.f, 1.f));
            gameObjects.emplace(pointLight.getId(), std::move(pointLight));
        }

        
    }

}