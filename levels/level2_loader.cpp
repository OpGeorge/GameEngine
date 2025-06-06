#include "level2_loader.hpp"


#include "../gen_game_object.hpp"
#include "../coreV/gen_texture.hpp"
#include "../coreV/gen_model.hpp"
#include "node_layouts.hpp"
#include <memory>

namespace gen {

    glm::vec3 Level2Loader::startingCameraPos{};

    glm::vec3 Level2Loader::getStartingCameraPos() {
        return startingCameraPos;
    }

    void Level2Loader::loadLevel2(GenDevice& genDevice, GenGameObject::Map& gameObjects) {
        
        startingCameraPos = glm::vec3(-20.f, 0.0f, 0.f);

        auto texture = std::make_shared<GenTexture>(genDevice, "textures/vaseTexture.png");

        std::shared_ptr<GenModel> genModel = GenModel::createModelFromFile(genDevice, "objectmodels/models/smooth_vase.obj");
        auto vase = GenGameObject::createGameObject();
        vase.model = genModel;
        vase.texture = texture;
        vase.type = ObjectType::Player;
        vase.transform.translation = { -20.0f, -2.0f, 2.5f };
        vase.transform.rotation.y = glm::radians(180.f);
        vase.transform.scale = glm::vec3(3.f);
        vase.soundDisc = std::make_unique<SoundDiscComponent>();
        vase.soundDisc->radius = 1.0f;
        vase.soundDisc->visible = true;
        vase.soundDisc->isPlayerControlled = true;
        vase.tag = "level2_player";
        gameObjects.emplace(vase.getId(), std::move(vase));

        //// Repeat for other objects like flat vase, cube, lights, etc.
        //genModel = GenModel::createModelFromFile(genDevice, "objectmodels/models/flat_vase.obj");
        //auto flatVase = GenGameObject::createGameObject();
        //flatVase.model = genModel;
        //flatVase.transform.translation = { 1.f,0.0f,0.f };
        //flatVase.transform.scale = glm::vec3(3.f);
        //flatVase.type = ObjectType::NPC;
        //flatVase.npcBehavior = std::make_unique<NPCBehaviorComponent>();
        //flatVase.tag = "level1_npc_1";
        //gameObjects.emplace(flatVase.getId(), std::move(flatVase)); // make sure the move has a valid pointer and a not null obj

        //genModel = GenModel::createModelFromFile(genDevice, "objectmodels/models/colored_cube.obj");
        //auto coloredCube = GenGameObject::createGameObject();
        //coloredCube.model = genModel;
        //coloredCube.transform.translation = { 0.f,-.5f,0.f };
        //coloredCube.transform.scale = glm::vec3(0.5f);
        //gameObjects.emplace(coloredCube.getId(), std::move(coloredCube));

        //ground
        genModel = GenModel::createModelFromFile(genDevice, "objectmodels/models/quad.obj");

        auto surface = GenGameObject::createGameObject();
        surface.model = genModel;
        surface.transform.translation = { -20.f,0.0f,-3.5f };
        surface.transform.scale = glm::vec3(8.f);
        gameObjects.emplace(surface.getId(), std::move(surface));



        //addNodeCluster1(genDevice, gameObjects);
        addNodeCluster2(genDevice, gameObjects,-20.f);

        { // lumina alba din cub
            auto pointLight = GenGameObject::makePointLight(4.2f,0.1f);
            pointLight.transform.translation = glm::vec3{ -20.0f,-5.5f,.0f };
            pointLight.type = ObjectType::Light;
            gameObjects.emplace(pointLight.getId(), std::move(pointLight));
        }


        ////point lights declaration
        //std::vector<glm::vec3> lightColors{
        //    {1.f, .1f, .1f},
        //    {.1f, .1f, 1.f},
        //    {.1f, 1.f, .1f},
        //    {1.f, 1.f, .1f},
        //    {.1f, 1.f, 1.f},
        //    {1.f, 1.f, 1.f}  //
        //};

        //for (int i = 0; i < lightColors.size(); i++) {
        //    auto pointLight = GenGameObject::makePointLight(0.2f);
        //    pointLight.type = ObjectType::Light;
        //    pointLight.color = lightColors[i];
        //    auto rotateLight = glm::rotate(glm::mat4(1.f), (i * glm::two_pi<float>()) / lightColors.size(), { 0.f,-1.f,0.f });
        //    pointLight.transform.translation = glm::vec3(rotateLight * glm::vec4(-1.f, -1.f, -1.f, 1.f));
        //    gameObjects.emplace(pointLight.getId(), std::move(pointLight));
        //}

    }

}