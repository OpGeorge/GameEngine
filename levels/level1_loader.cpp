#include "level1_loader.hpp"


#include "../gen_game_object.hpp"
#include "../coreV/gen_texture.hpp"
#include "../coreV/gen_model.hpp"
#include "node_layouts.hpp"
#include <memory>

namespace gen {


    glm::vec3 Level1Loader::startingCameraPos{};
    
    glm::vec3 Level1Loader::getStartingCameraPos() {
        return startingCameraPos;
    }

    void Level1Loader::loadLevel1(GenDevice& genDevice, GenGameObject::Map& gameObjects) {
        
        startingCameraPos = glm::vec3(0.f, 0.0f, 1.f);

        auto texture = std::make_shared<GenTexture>(genDevice, "textures/red.png");
        auto texture2 = std::make_shared<GenTexture>(genDevice, "textures/space-cruiser.png");
        auto texture3 = std::make_shared<GenTexture>(genDevice, "textures/floor.png");
        auto texture4 = std::make_shared<GenTexture>(genDevice, "textures/blue.png");

        std::shared_ptr<GenModel> genModel = GenModel::createModelFromFile(genDevice, "objectmodels/models/Player Model.obj");
        auto vase = GenGameObject::createGameObject();
        vase.model = genModel;
        vase.texture = texture;
        vase.type = ObjectType::Player;
        vase.transform.translation = { 0.0f, -2.0f, 2.5f };
        vase.transform.rotation.y = glm::radians(180.f);
        vase.transform.scale = glm::vec3(0.3f);
        vase.soundDisc = std::make_unique<SoundDiscComponent>();
        vase.soundDisc->radius = 1.0f;
        vase.soundDisc->visible = true;
        vase.soundDisc->isPlayerControlled = true; 
        vase.tag = "level1_player";

        gameObjects.emplace(vase.getId(), std::move(vase));

        // Repeat for other objects like flat vase, cube, lights, etc.
        genModel = GenModel::createModelFromFile(genDevice, "objectmodels/models/Player Model.obj");
        auto flatVase = GenGameObject::createGameObject();
        flatVase.model = genModel;
        flatVase.transform.translation = { 1.f,0.0f,-7.f };
        flatVase.transform.scale = glm::vec3(0.4f);
        flatVase.texture = texture4;
        flatVase.type = ObjectType::NPC;
        flatVase.npcBehavior = std::make_unique<NPCBehaviorComponent>();
        flatVase.tag = "level1_npc_1";
        gameObjects.emplace(flatVase.getId(), std::move(flatVase)); // make sure the move has a valid pointer and a not null obj

        genModel = GenModel::createModelFromFile(genDevice, "objectmodels/models/colored_cube.obj");
        auto coloredCube = GenGameObject::createGameObject();
        coloredCube.model = genModel;
        coloredCube.transform.translation = { 0.f,-.5f,-15.f };
        coloredCube.transform.scale = glm::vec3(0.3f);
        coloredCube.goal = std::make_unique<GoalComponent>();
        coloredCube.type = ObjectType::Goal;
        gameObjects.emplace(coloredCube.getId(), std::move(coloredCube));

        auto pointLightGoal = GenGameObject::makePointLight(4.5f, 0.1f, glm::vec3(1.0f, 0.843f, 0.0f));
        pointLightGoal.transform.translation = glm::vec3{ 0.0f,-2.5f,-15.0f };
        pointLightGoal.type = ObjectType::Light;
        gameObjects.emplace(pointLightGoal.getId(), std::move(pointLightGoal));

        genModel = GenModel::createModelFromFile(genDevice, "objectmodels/models/quad.obj");
        auto surface2 = GenGameObject::createGameObject();
        surface2.model = genModel;
        surface2.texture = texture3;
        surface2.transform.translation = { 0.f,0.0f,-14.5f };
        surface2.transform.scale = glm::vec3(2.0f, 1.0f, 3.f);
        gameObjects.emplace(surface2.getId(), std::move(surface2));

        genModel = GenModel::createModelFromFile(genDevice, "objectmodels/models/quad.obj");
        auto ceilingGoal = GenGameObject::createGameObject();
        ceilingGoal.model = genModel;
        ceilingGoal.texture = texture3;
        ceilingGoal.transform.translation = { 0.f,-3.5f,-14.5f };
        ceilingGoal.transform.scale = glm::vec3(2.0f, 1.0f, 3.f);
        ceilingGoal.type = ObjectType::Wall;
        gameObjects.emplace(ceilingGoal.getId(), std::move(ceilingGoal));


        //ground
        genModel = GenModel::createModelFromFile(genDevice, "objectmodels/models/quad.obj");

        auto surface = GenGameObject::createGameObject();
        surface.model = genModel;
        surface.transform.translation = { 0.f,0.0f,-3.5f };
        surface.texture = texture3;
        surface.transform.scale = glm::vec3(8.f);
        gameObjects.emplace(surface.getId(), std::move(surface));

 

        addNodeCluster1(genDevice, gameObjects);
        //addNodeCluster2(genDevice, gameObjects);

        auto pointLight = GenGameObject::makePointLight(4.2f, 0.1f);
        pointLight.transform.translation = glm::vec3{ 0.0f,-4.5f,2.0f };
        pointLight.type = ObjectType::Light;
        gameObjects.emplace(pointLight.getId(), std::move(pointLight));

        auto pointLight2 = GenGameObject::makePointLight(4.2f, 0.1f);
        pointLight2.transform.translation = glm::vec3{ 6.0f,-4.5f,2.0f };
        pointLight2.type = ObjectType::Light;
        gameObjects.emplace(pointLight2.getId(), std::move(pointLight2));

        auto pointLight3 = GenGameObject::makePointLight(4.2f, 0.1f);
        pointLight3.transform.translation = glm::vec3{ -6.0f,-4.5f,2.0f };
        pointLight3.type = ObjectType::Light;
        gameObjects.emplace(pointLight3.getId(), std::move(pointLight3));

        // randu 2

        auto pointLight4 = GenGameObject::makePointLight(4.2f, 0.1f);
        pointLight4.transform.translation = glm::vec3{ 0.0f,-4.5f,-3.0f };
        pointLight4.type = ObjectType::Light;
        gameObjects.emplace(pointLight4.getId(), std::move(pointLight4));

        auto pointLight5 = GenGameObject::makePointLight(4.2f, 0.1f);
        pointLight5.transform.translation = glm::vec3{ 6.0f,-4.5f,-3.0f };
        pointLight5.type = ObjectType::Light;
        gameObjects.emplace(pointLight5.getId(), std::move(pointLight5));

        auto pointLight6 = GenGameObject::makePointLight(4.2f, 0.1f);
        pointLight6.transform.translation = glm::vec3{ -6.0f,-4.5f,-3.0f };
        pointLight6.type = ObjectType::Light;
        gameObjects.emplace(pointLight6.getId(), std::move(pointLight6));

        //randu 3

        auto pointLight7 = GenGameObject::makePointLight(4.2f, 0.1f);
        pointLight7.transform.translation = glm::vec3{ 0.0f,-4.5f,-7.0f };
        pointLight7.type = ObjectType::Light;
        gameObjects.emplace(pointLight7.getId(), std::move(pointLight7));

        auto pointLight8 = GenGameObject::makePointLight(4.2f, 0.1f);
        pointLight8.transform.translation = glm::vec3{ 6.0f,-4.5f,-7.0f };
        pointLight8.type = ObjectType::Light;
        gameObjects.emplace(pointLight8.getId(), std::move(pointLight8));

        auto pointLight9 = GenGameObject::makePointLight(4.2f, 0.1f);
        pointLight9.transform.translation = glm::vec3{ -6.0f,-4.5f,-7.0f };
        pointLight9.type = ObjectType::Light;
        gameObjects.emplace(pointLight9.getId(), std::move(pointLight9));



        //auto pointLight3 = GenGameObject::makePointLight(4.2f, 0.1f);
        //pointLight3.transform.translation = glm::vec3{ -6.0f,-3.5f,2.0f };
        //pointLight3.type = ObjectType::Light;
        //gameObjects.emplace(pointLight3.getId(), std::move(pointLight3));



        //WALLS
        genModel = GenModel::createModelFromFile(genDevice, "objectmodels/models/quad.obj");
        auto wallR = GenGameObject::createGameObject();
        wallR.model = genModel;
        wallR.transform.translation = { -7.f,-2.5f,-3.5f };
        wallR.texture = texture3;
        wallR.transform.rotation = glm::vec3(glm::radians(-90.0f), glm::radians(90.0f), 0.0f);
        wallR.transform.scale = glm::vec3(8.f, 1.0f,3.5f);
        wallR.type = ObjectType::Wall;
        gameObjects.emplace(wallR.getId(), std::move(wallR));
        //WALLS
        genModel = GenModel::createModelFromFile(genDevice, "objectmodels/models/quad.obj");
        auto wallL = GenGameObject::createGameObject();
        wallL.model = genModel;
        wallL.transform.translation = { 7.f,-2.5f,-3.5f };
        wallL.texture = texture3;
        wallL.transform.rotation = glm::vec3(glm::radians(90.0f), glm::radians(90.0f), 0.0f);
        wallL.transform.scale = glm::vec3(8.f, 1.0f, 3.5f);
        wallL.type = ObjectType::Wall;
        gameObjects.emplace(wallL.getId(), std::move(wallL));

        genModel = GenModel::createModelFromFile(genDevice, "objectmodels/models/quad.obj");
        auto ceiling = GenGameObject::createGameObject();
        ceiling.model = genModel;
        ceiling.transform.translation = { 0.f,-3.5f,-3.5f };
        ceiling.texture = texture3;
        ceiling.transform.rotation = glm::vec3(glm::radians(.0f), glm::radians(180.0f), 0.0f);
        ceiling.transform.scale = glm::vec3(8.f,1.0f,8.f);
        ceiling.type = ObjectType::Wall;
        gameObjects.emplace(ceiling.getId(), std::move(ceiling));

        genModel = GenModel::createModelFromFile(genDevice, "objectmodels/models/quad.obj");
        auto facingWall = GenGameObject::createGameObject();
        facingWall.model = genModel;
        facingWall.transform.translation = {10.f,-2.5f,-11.5f };
        facingWall.texture = texture3;
        facingWall.transform.rotation = glm::vec3(glm::radians(-90.0f), glm::radians(0.0f), glm::radians(0.0f));
        facingWall.transform.scale = glm::vec3(8.f, 1.0f, 3.5f);
        facingWall.type = ObjectType::Wall;
        gameObjects.emplace(facingWall.getId(), std::move(facingWall));

        genModel = GenModel::createModelFromFile(genDevice, "objectmodels/models/quad.obj");
        auto facingWall2 = GenGameObject::createGameObject();
        facingWall2.model = genModel;
        facingWall2.transform.translation = { -10.f,-2.5f,-11.5f };
        facingWall2.texture = texture3;
        facingWall2.transform.rotation = glm::vec3(glm::radians(-90.0f), glm::radians(0.0f), glm::radians(0.0f));
        facingWall2.transform.scale = glm::vec3(8.f, 1.0f, 3.5f);
        facingWall2.type = ObjectType::Wall;
        gameObjects.emplace(facingWall2.getId(), std::move(facingWall2));

        genModel = GenModel::createModelFromFile(genDevice, "objectmodels/models/quad.obj");
        auto facingWall3 = GenGameObject::createGameObject();
        facingWall3.model = genModel;
        facingWall3.transform.translation = { 0.f,-2.5f,-17.5f };
        facingWall3.texture = texture3;
        facingWall3.transform.rotation = glm::vec3(glm::radians(-90.0f), glm::radians(0.0f), glm::radians(0.0f));
        facingWall3.transform.scale = glm::vec3(8.f, 1.0f, 3.5f);
        facingWall3.type = ObjectType::Wall;
        gameObjects.emplace(facingWall3.getId(), std::move(facingWall3));

        genModel = GenModel::createModelFromFile(genDevice, "objectmodels/models/quad.obj");
        auto wallLGoal = GenGameObject::createGameObject();
        wallLGoal.model = genModel;
        wallLGoal.transform.translation = { 2.f,-2.5f,-19.5f };
        wallLGoal.texture = texture3;
        wallLGoal.transform.rotation = glm::vec3(glm::radians(90.0f), glm::radians(90.0f), 0.0f);
        wallLGoal.transform.scale = glm::vec3(8.f, 1.0f, 3.5f);
        wallLGoal.type = ObjectType::Wall;
        gameObjects.emplace(wallLGoal.getId(), std::move(wallLGoal));

        genModel = GenModel::createModelFromFile(genDevice, "objectmodels/models/quad.obj");
        auto wallRGoal = GenGameObject::createGameObject();
        wallRGoal.model = genModel;
        wallRGoal.transform.translation = { -2.f,-2.5f,-19.5f };
        wallRGoal.texture = texture3;
        wallRGoal.transform.rotation = glm::vec3(glm::radians(-90.0f), glm::radians(90.0f), 0.0f);
        wallRGoal.transform.scale = glm::vec3(8.f, 1.0f, 3.5f);
        wallRGoal.type = ObjectType::Wall;
        gameObjects.emplace(wallRGoal.getId(), std::move(wallRGoal));

        genModel = GenModel::createModelFromFile(genDevice, "objectmodels/models/quad.obj");
        auto backWall = GenGameObject::createGameObject();
        backWall.model = genModel;
        backWall.transform.translation = { 0.f,-2.5f,4.5f };
        backWall.texture = texture3;
        backWall.transform.rotation = glm::vec3(glm::radians(-90.0f), glm::radians(180.0f), glm::radians(0.0f));
        backWall.transform.scale = glm::vec3(8.f, 1.0f, 3.5f);
        backWall.type = ObjectType::Wall;
        gameObjects.emplace(backWall.getId(), std::move(backWall));

        //Pillar
        genModel = GenModel::createModelFromFile(genDevice, "objectmodels/models/colored_cube.obj");
        auto pillar = GenGameObject::createGameObject();
        pillar.model = genModel;
        pillar.texture = texture3;
        pillar.transform.translation = { 5.f,-0.f,-3.5f };
        pillar.transform.scale = glm::vec3(3.5f, 1.f, 2.f);
        pillar.type = ObjectType::Wall;
        gameObjects.emplace(pillar.getId(), std::move(pillar));

        //Pillar
        genModel = GenModel::createModelFromFile(genDevice, "objectmodels/models/colored_cube.obj");
        auto pillar2 = GenGameObject::createGameObject();
        pillar2.model = genModel;
        pillar2.texture = texture3;
        pillar2.transform.translation = { -5.f,-0.f,-3.5f };
        pillar2.transform.scale = glm::vec3(3.5f, 1.f, 2.f);
        pillar2.type = ObjectType::Wall;
        gameObjects.emplace(pillar2.getId(), std::move(pillar2));

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