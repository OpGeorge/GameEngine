#include "gen_appctrl.hpp"

#include "coreV/gen_buffer.hpp"
#include "coreV/wireframe_render_system.hpp"
#include "coreV/simple_render_system.hpp"
#include "coreV/point_light_system.hpp"
#include "coreV/gen_texture_swapper.hpp"

#include "gen_camera.hpp"
#include "keyboard_movement_controller.hpp"
#include "gen_obj_movement.hpp"
#include "gen_npc_controller.hpp"

#include "levels/mem_test_loader.hpp"
#include "levels/mem_test_npc_grid_loader.hpp"

#include "levels/level1_loader.hpp"
#include "levels/level2_loader.hpp"


#define GLM_ENABLE_EXPERIMENTAL

#define GLM_FORCE_RADIENTS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include <glm/glm.hpp>
#include <glm/gtc/constants.hpp>
#include <vector>
#include <iostream>
#include <queue>



#include <stdexcept>
#include <array>
#include <cassert>
#include <chrono>


namespace gen {




    AppCtrl::AppCtrl() {

        Level1Loader::loadLevel1(genDevice, gameObjects);
        for (const auto& [id, obj] : gameObjects) {
            if (!obj.tag.empty() && (obj.type == ObjectType::Player || obj.type == ObjectType::NPC)) {
                initialTransformsLevel1[obj.tag] = obj.transform;
            }
        }
        Level2Loader::loadLevel2(genDevice, gameObjects);
        for (const auto& [id, obj] : gameObjects) {
            if (!obj.tag.empty() && (obj.type == ObjectType::Player || obj.type == ObjectType::NPC)) {
                initialTransformsLevel2[obj.tag] = obj.transform;
            }
        }

        //MemTestLoader::addNodes1000(genDevice, gameObjects);
        //MemTestNPCGridLoader::addNPCs100(genDevice, gameObjects);


        activeGameObjects = &gameObjects;

        

        std::srand(static_cast<unsigned>(std::time(nullptr)));

        const int maxObjects = static_cast<int>(gameObjects.size()) + 10;  // Add padding

        globalPool = GenDescriptorPool::Builder(genDevice)
            .setMaxSets(2000)
            .addPoolSize(VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 2000)
            .addPoolSize(VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 2000)
            .build();

    }

    AppCtrl::~AppCtrl() {


    }

    void AppCtrl::run() {
        std::shared_ptr<GenTexture> fallbackTexture = std::make_shared<GenTexture>(genDevice, "textures/white.png");

        bool textureSwapPending = false;
        std::string pendingTexturePath;



        int currentFrameNumber = 0;

        std::vector<std::unique_ptr<GenBuffer>> uboBuffers(GenSwapChain::MAX_FRAMES_IN_FLIGHT);
        std::unordered_map<GenGameObject::id_t, std::vector<std::unique_ptr<GenBuffer>>> textureToggleBuffers;

        for (int i = 0; i < GenSwapChain::MAX_FRAMES_IN_FLIGHT; i++) {
            uboBuffers[i] = std::make_unique<GenBuffer>(
                genDevice,
                sizeof(GlobalUbo),
                1,
                VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT,
                VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);
            uboBuffers[i]->map();


        }

        auto globalSetLayout = GenDescriptorSetLayout::Builder(genDevice)
            .addBinding(0, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT)
            .addBinding(1, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, VK_SHADER_STAGE_FRAGMENT_BIT)
            .addBinding(2, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, VK_SHADER_STAGE_FRAGMENT_BIT)
            .build();



        std::vector<VkDescriptorSet> globalDescriptorSets(GenSwapChain::MAX_FRAMES_IN_FLIGHT);
        std::array<std::unordered_map<GenGameObject::id_t, VkDescriptorSet>, GenSwapChain::MAX_FRAMES_IN_FLIGHT> objectDescriptorSets;

        for (int frameIndex = 0; frameIndex < GenSwapChain::MAX_FRAMES_IN_FLIGHT; frameIndex++) {
            for (auto& [id, obj] : *activeGameObjects) {
                VkDescriptorSet descriptorSet;

                // UBO for projection/view
                VkDescriptorBufferInfo uboInfo = uboBuffers[frameIndex]->descriptorInfo();

                // Texture binding logic
                const bool hasRealTexture = obj.texture != nullptr;
                VkDescriptorImageInfo imageInfo = hasRealTexture
                    ? obj.texture->descriptorInfo()
                    : fallbackTexture->descriptorInfo();

                int useTextureFlag = hasRealTexture ? 1 : 0;

                // Allocate per-object texture toggle buffer if needed
                if (textureToggleBuffers.find(id) == textureToggleBuffers.end()) {
                    std::vector<std::unique_ptr<GenBuffer>> buffers;

                    for (int i = 0; i < GenSwapChain::MAX_FRAMES_IN_FLIGHT; i++) {
                        auto buffer = std::make_unique<GenBuffer>(
                            genDevice,
                            sizeof(int),
                            1,
                            VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT,
                            VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT
                        );
                        buffer->map();
                        buffers.push_back(std::move(buffer));
                    }

                    textureToggleBuffers.emplace(id, std::move(buffers));
                }

                // Write the flag for this object for this frame
                textureToggleBuffers[id][frameIndex]->writeToBuffer(&useTextureFlag);
                VkDescriptorBufferInfo flagInfo = textureToggleBuffers[id][frameIndex]->descriptorInfo();

                // Build descriptor set
                auto writer = GenDescriptorWriter(*globalSetLayout, *globalPool)
                    .writeBuffer(0, &uboInfo)
                    .writeImage(1, &imageInfo)
                    .writeBuffer(2, &flagInfo);        // tell shader whether to sample

                if (!writer.build(descriptorSet)) {
                    std::cerr << "Descriptor set allocation failed. Pool may be exhausted.\n";

                    vkDeviceWaitIdle(genDevice.device());

                    // Safely destroy old pool and build a new one
                    globalPool = GenDescriptorPool::Builder(genDevice)
                        .setMaxSets(2000)  // or increase this number
                        .addPoolSize(VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 1000)
                        .addPoolSize(VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 1000)
                        .build();

                    auto retryWriter = GenDescriptorWriter(*globalSetLayout, *globalPool)
                        .writeBuffer(0, &uboInfo)
                        .writeImage(1, &imageInfo)
                        .writeBuffer(2, &flagInfo);

                    if (!retryWriter.build(descriptorSet)) {
                        throw std::runtime_error("Failed to rebuild descriptor set even after pool reset");
                    }
                }

                objectDescriptorSets[frameIndex][id] = descriptorSet;



                if (globalDescriptorSets[frameIndex] == VK_NULL_HANDLE) {
                    globalDescriptorSets[frameIndex] = descriptorSet;
                }
            }
        }

        TextureSwapper textureSwapper(
            genDevice,
            *globalSetLayout,
            *globalPool,
            uboBuffers,
            textureToggleBuffers,
            objectDescriptorSets);

        SimpleRenderSystem simpleRenderSystem{
            genDevice,
            genRenderer.getSwapChainRenderPass(),
            globalSetLayout->getDescriptorSetLayout()
        };

        WireframeRenderSystem wireframeRenderSystem{
            genDevice,
            genRenderer.getSwapChainRenderPass(),
            globalSetLayout->getDescriptorSetLayout()
        };

        PointLightSystem pointLightSystem{
            genDevice,
            genRenderer.getSwapChainRenderPass(),
            globalSetLayout->getDescriptorSetLayout()
        };



        GenCamera camera{};
        camera.setViewDirection(glm::vec3(0.0f, -0.5f, -2.5f), glm::vec3(0.0f, 0.0f, -1.0f));

        auto currentTime = std::chrono::high_resolution_clock::now();

        auto viewerObject = GenGameObject::createGameObject();
        viewerObject.transform.translation.z = -2.5f;
        viewerObject.transform.translation.y = -.5f;

        std::vector<GenGameObject> cameraStand;
        {
            auto cam1 = GenGameObject::createGameObject();
            cam1.type = ObjectType::Camera;
            cam1.transform.translation = { 0.f, -1.3f, 3.f };
            cam1.transform.rotation = { 0.0f,60.0f,0.0f };

            auto cam2 = GenGameObject::createGameObject();
            cam2.type = ObjectType::Camera;
            cam2.transform.translation = { 0.f, -1.3f, -2.5f };
            cam2.transform.rotation = { 0.f, -glm::radians(10.f), 0.f };

            auto cam3 = GenGameObject::createGameObject();
            cam3.transform.translation = { -17.f, -1.5f, 3.f };
            cam3.transform.rotation = { 0.f, -glm::radians(180.f), 0.f };
            cam3.type = ObjectType::Camera;

            auto cam4 = GenGameObject::createGameObject();
            cam4.transform.translation = { -20.f, -5.5f, -2.f };
            cam4.transform.rotation = { -glm::radians(25.0f),0.0f, 0.0f };

            cameraStand.push_back(std::move(cam1));
            cameraStand.push_back(std::move(cam2));
            cameraStand.push_back(std::move(cam3));
            cameraStand.push_back(std::move(cam4));
        }

        std::vector<GenGameObject*> controllableObjects;
        std::vector<GenGameObject*> controllableNPCs;

        for (auto& [id, obj] : *activeGameObjects) {
            if (obj.type == ObjectType::Player) {
                controllableObjects.push_back(&obj);
                std::cout << controllableObjects.size() << "\n";
            }
            if (obj.type == ObjectType::NPC) {
                controllableNPCs.push_back(&obj);
            }
        }

        GameObjectMovementController objectController;


        //test harcoded target
        static glm::vec3 target = { 2.0f,0.0f,-3.0f };
        NPCMovementController npcController;


        int activeCameraIndex = 0;
        int activeObjectIndex = controllableObjects.size()-1;
        currentLevel = 0;
        KeyboardMovementController cameraController{};
        const float MAX_FRAME_TIME = 165.f;

        bool followPlayerCamera = false;
        float cameraPitch = -1.5f;
        bool firstFollowBind = true;
        float elapsedTime = 0.f;

        while (!genWindow.shouldClose()) {
            glfwPollEvents();



            static bool tabPressedLastFrame = false;
            if (glfwGetKey(genWindow.getGLFWwindow(), GLFW_KEY_TAB) == GLFW_PRESS) {
                if (!tabPressedLastFrame) {
                    activeCameraIndex = (activeCameraIndex + 1) % cameraStand.size();
                    followPlayerCamera = false;
                }
                tabPressedLastFrame = true;
            }
            else {
                tabPressedLastFrame = false;
            }

            
            auto newTime = std::chrono::high_resolution_clock::now();
            float frameTime = std::chrono::duration<float>(newTime - currentTime).count();
            elapsedTime += frameTime;
            currentTime = newTime;
            frameTime = glm::min(frameTime, MAX_FRAME_TIME);

            // bind camera to player logic

            if (followPlayerCamera && !controllableObjects.empty()) {
                GenGameObject* activeObj = controllableObjects[activeObjectIndex];

                cameraController.processMouseLookToggle(genWindow.getGLFWwindow());
                
                if (cameraController.mouseLookEnabled) {
                    double mouseX, mouseY;
                    glfwGetCursorPos(genWindow.getGLFWwindow(), &mouseX, &mouseY);

                    double deltaX = mouseX - cameraController.lastMouseX;
                    double deltaY = mouseY - cameraController.lastMouseY;
                    cameraController.lastMouseX = mouseX;
                    cameraController.lastMouseY = mouseY;

                    // Update player yaw (horizontal)
                    activeObj->transform.rotation.y += static_cast<float>(deltaX) * cameraController.mouseSensitivity;

                    // Update camera pitch (vertical)
                    cameraPitch -= static_cast<float>(deltaY) * cameraController.mouseSensitivity;
                    //cameraPitch += cameraController.mouseSensitivity;
                    cameraPitch = glm::clamp(cameraPitch, -1.5f, 1.5f);  // optional clamp


                }

                glm::vec3 playerPos = activeObj->transform.translation;
                glm::vec3 cameraPos = playerPos + glm::vec3(0.f, -1.5f, 0.f);  // raise camera above player (Y-down)
                glm::vec3 cameraRot = glm::vec3(cameraPitch, activeObj->transform.rotation.y, 0.f);
                camera.setViewYXZ(cameraPos, cameraRot);


              
            }
            else {
                cameraController.processMouseLookToggle(genWindow.getGLFWwindow());
                cameraController.updateCameraViewFromMouse(genWindow.getGLFWwindow(), cameraStand[activeCameraIndex]);
                cameraController.moveInPlaneXZ(genWindow.getGLFWwindow(), frameTime, cameraStand[activeCameraIndex]);
                camera.setViewYXZ(cameraStand[activeCameraIndex].transform.translation, cameraStand[activeCameraIndex].transform.rotation);
            }
            // end of bind camera to player logic
           
            float aspect = genRenderer.getAspectratio();
            camera.setPerspectiveProjection(glm::radians(50.f), aspect, 0.1f, 50.f);

            //end of camera logic (above is the ratio and perspective logic)

            // apply physics to player entities logic function
            logicManager.update(frameTime, *activeGameObjects);

            // Reset nodes after cooldown expires
            for (auto& [id, obj] : *activeGameObjects) {
                if (obj.type != ObjectType::Node || !obj.node) continue;

                auto cooldownIt = nodeCooldowns.find(id);
                if (cooldownIt != nodeCooldowns.end() && elapsedTime >= cooldownIt->second) {
                    auto& node = *obj.node;

                    node.color = NodeComponent::NodeColor::White;
                    node.lastColorApplied = NodeComponent::NodeColor::White;
                    node.activated = false;
                    node.hasPropagated = false;
                    node.parentId = -1;

                    auto whiteTexture = getCachedTexture("textures/white.png");
                    if (obj.texture != whiteTexture) {
                        obj.texture = whiteTexture;
                        obj.textureDirty = true;
                    }

                    nodeCooldowns.erase(cooldownIt);
                }
            }


            if (controllableNPCs.size() > 0) {
                // Update NPC behavior AI
                auto sharedWhiteTexture = getCachedTexture("textures/white.png");

                for (GenGameObject* npc : controllableNPCs) {
                    npcController.updateAI(frameTime, *npc, *activeGameObjects, sharedWhiteTexture,this);
                }
            }
            

            // player object select
            static bool switchPressedLastFrame = false;
            if (glfwGetKey(genWindow.getGLFWwindow(), GLFW_KEY_C) == GLFW_PRESS) {
                if (!switchPressedLastFrame && !controllableObjects.empty()) {
                    activeObjectIndex = (activeObjectIndex + 1) % controllableObjects.size();
                    followPlayerCamera = true;
                    firstFollowBind = true;
                    std::cout << "Camera now following player index: " << activeObjectIndex << "\n";
                }
                switchPressedLastFrame = true;
            }
            else {
                switchPressedLastFrame = false;
            }

            ///// goal detection 
            //for (auto& [id, obj] : *activeGameObjects) {
            //    if (!obj.goal) continue;

            //    float distanceSq = glm::distance2(
            //        controllableObjects[activeObjectIndex]->transform.translation,
            //        obj.transform.translation
            //    );

            //    float radiusSq = obj.goal->triggerRadius * obj.goal->triggerRadius;

            //    static bool ePressedLastFrame = false;
            //    bool ePressed = glfwGetKey(genWindow.getGLFWwindow(), GLFW_KEY_E) == GLFW_PRESS;

            //    if (distanceSq < radiusSq && ePressed && !ePressedLastFrame) {
            //        // Trigger the same behavior as pressing 'C'
            //        activeObjectIndex = (activeObjectIndex + 1) % controllableObjects.size();
            //        followPlayerCamera = true;
            //        firstFollowBind = true;

            //        std::cout << "Goal reached! Switching camera to follow player index: "
            //            << activeObjectIndex << "\n";
            //        currentLevel++;
            //        if (currentLevel % 3 == 2) {
            //        // trigger completion state
            //            std::cout << "FINAL Goal reached! "
            //                << activeObjectIndex << "\n";
            //        }
            //        
            //    }

            //    ePressedLastFrame = ePressed;
            //}


            /// goal detection 
            GenGameObject* closestGoal = nullptr;
            float closestGoalDistanceSq = 0.0f;
            bool foundAnyGoal = false;

            glm::vec3 playerPos = controllableObjects[activeObjectIndex]->transform.translation;

            for (auto& [id, obj] : *activeGameObjects) {
                if (!obj.goal) continue;

                float distanceSq = glm::distance2(playerPos, obj.transform.translation);
                float radiusSq = obj.goal->triggerRadius * obj.goal->triggerRadius;

                if (distanceSq < radiusSq) {
                    if (!foundAnyGoal || distanceSq < closestGoalDistanceSq) {
                        closestGoal = &obj;
                        closestGoalDistanceSq = distanceSq;
                        foundAnyGoal = true;
                    }
                }
            }

            // Static state tracking
            static bool ePressedLastFrame = false;
            bool ePressed = glfwGetKey(genWindow.getGLFWwindow(), GLFW_KEY_E) == GLFW_PRESS;

            // If a valid goal was found and E is freshly pressed
            if (foundAnyGoal && closestGoal && ePressed && !ePressedLastFrame) {
                
                currentLevel++;
                if (currentLevel == 3) {
                    currentLevel = 0;
                }

                if (currentLevel % 3 > 1) {
                    std::cout << "FINAL Goal reached! " << activeObjectIndex << "\n";
                    
                    activeCameraIndex = cameraStand.size() - 1;
                    followPlayerCamera = false;
                    resetLevelTransforms(initialTransformsLevel1);
                    resetLevelTransforms(initialTransformsLevel2);
                    currentLevel = 2;
                }
                else {
                    activeObjectIndex = (activeObjectIndex + 1) % controllableObjects.size();
                    followPlayerCamera = true;
                    firstFollowBind = true;

                    std::cout << "Goal reached! Switching camera to follow player index: "
                        << activeObjectIndex << "\n";
                   
                }

                
            }

            ePressedLastFrame = ePressed;




            static bool rPressedLastFrame = false;
            if (glfwGetKey(genWindow.getGLFWwindow(), GLFW_KEY_R) == GLFW_PRESS) {
                if (!rPressedLastFrame) {
                    if(currentLevel % 3 == 0){
                        std::cout << "Resetting Level 1 positions...\n";
                        resetLevelTransforms(initialTransformsLevel1);
                    }
                    else if (currentLevel % 3 == 1) {
                        std::cout << "Resetting Level 2 positions...\n";
                        resetLevelTransforms(initialTransformsLevel2);
                    }

                }
                rPressedLastFrame = true;
            }
            else {
                rPressedLastFrame = false;
            }

            // control Player object logic + sound propgation form player logic
            if (!controllableObjects.empty()) {
                GenGameObject* activeObj = controllableObjects[activeObjectIndex];

                // Always move the object
                objectController.moveInPlaneXZ(genWindow.getGLFWwindow(), frameTime, *activeObj);

                // Only allow Player-type to emit sound and trigger propagation
                if (activeObj->type == ObjectType::Player) {
                    updateNodeColorAndTextureFromPlayer(
                        *activeObj,
                        *activeGameObjects,
                        getCachedTexture("textures/red.png"),
                        getCachedTexture("textures/orange.png"),
                        getCachedTexture("textures/green.png"),
                        currentFrameNumber,
                        elapsedTime
                    );

                    for (auto& [id, obj] : *activeGameObjects) {
                        if (obj.type != ObjectType::Node || !obj.node || !obj.node->activated) continue;
                        if (!obj.node->hasPropagated && obj.node->color == NodeComponent::NodeColor::Red ||
                            obj.node->color == NodeComponent::NodeColor::Orange) {
                            propagationSystem.propagateFromNode(
                                obj,
                                gameObjects,
                                1.4f,
                                getCachedTexture("textures/orange.png"),
                                getCachedTexture("textures/green.png"),
                                currentFrameNumber,
                                lastTextureChangeFrame,
                                nodeCooldowns,
                                elapsedTime
                            );

                            obj.node->hasPropagated = true;

                        }
                    }
                }
            }

            // frame render with the applied above logic
            if (auto commandBuffer = genRenderer.beginFrame()) {

                int frameIndex = genRenderer.getFrameIndex();

                refreshObjectDescriptorsIfNeeded(
                    fallbackTexture,
                    uboBuffers,
                    textureToggleBuffers,
                    objectDescriptorSets,
                    *globalSetLayout,
                    *globalPool);

                FrameInfo frameInfo{
                    frameIndex,
                    frameTime,
                    commandBuffer,
                    camera,
                    globalDescriptorSets[frameIndex],
                    objectDescriptorSets[frameIndex],
                    *activeGameObjects
                };

                GlobalUbo ubo{};
                ubo.projection = camera.getProjcetion();
                ubo.view = camera.getView();
                ubo.inverseView = camera.getInverseView();
                pointLightSystem.update(frameInfo, ubo);

                uboBuffers[frameIndex]->writeToBuffer(&ubo);
                uboBuffers[frameIndex]->flush();

                genRenderer.beginSwachChainRenderPass(commandBuffer);

                simpleRenderSystem.renderGameObjects(frameInfo);
                pointLightSystem.render(frameInfo);
                wireframeRenderSystem.render(frameInfo);
                genRenderer.endSwachChainRenderPass(commandBuffer);
                genRenderer.endFrame();

                currentFrameNumber++;

            }
        }

        vkDeviceWaitIdle(genDevice.device());
    }


    void AppCtrl::refreshObjectDescriptorsIfNeeded(
        std::shared_ptr<GenTexture> fallbackTexture,
        std::vector<std::unique_ptr<GenBuffer>>& uboBuffers,
        std::unordered_map<GenGameObject::id_t, std::vector<std::unique_ptr<GenBuffer>>>& textureToggleBuffers,
        std::array<std::unordered_map<GenGameObject::id_t, VkDescriptorSet>, GenSwapChain::MAX_FRAMES_IN_FLIGHT>& objectDescriptorSets,
        GenDescriptorSetLayout& globalSetLayout,
        GenDescriptorPool& globalPool

    ) {
        for (int i = 0; i < GenSwapChain::MAX_FRAMES_IN_FLIGHT; i++) {
            for (auto& [id, obj] : *activeGameObjects) {
                if (!obj.textureDirty) continue;

                // Prevent fallback flickering: skip rebuild if texture not yet assigned
                if (!obj.texture) {
                    std::cerr << "[SKIP] Texture is still null for ID " << id <<" deferring descriptor rebuild.\n";
                    continue;
                }

                VkDescriptorSet descriptorSet;
                VkDescriptorBufferInfo uboInfo = uboBuffers[i]->descriptorInfo();

                int useTextureFlag = 1;  // texture is guaranteed valid
                VkDescriptorImageInfo imageInfo = obj.texture->descriptorInfo();

                if (textureToggleBuffers.find(id) == textureToggleBuffers.end()) {
                    std::vector<std::unique_ptr<GenBuffer>> buffers;
                    for (int j = 0; j < GenSwapChain::MAX_FRAMES_IN_FLIGHT; j++) {
                        auto buffer = std::make_unique<GenBuffer>(
                            genDevice,
                            sizeof(int),
                            1,
                            VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT,
                            VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT
                        );
                        buffer->map();
                        buffers.push_back(std::move(buffer));
                    }
                    textureToggleBuffers.emplace(id, std::move(buffers));
                }

                textureToggleBuffers[id][i]->writeToBuffer(&useTextureFlag);
                VkDescriptorBufferInfo flagInfo = textureToggleBuffers[id][i]->descriptorInfo();

                auto writer = GenDescriptorWriter(globalSetLayout, globalPool)
                    .writeBuffer(0, &uboInfo)
                    .writeImage(1, &imageInfo)
                    .writeBuffer(2, &flagInfo);

                std::cout << "[DESCRIPTOR REBUILD] Object ID: " << id
                    << " | Frame: " << i
                    << " | Texture Ptr: " << obj.texture.get()
                    << " | Dirty: " << obj.textureDirty
                    << "\n";

                if (!writer.build(descriptorSet)) {
                    throw std::runtime_error("Failed to rebuild descriptor set in refresh");
                }

                objectDescriptorSets[i][id] = descriptorSet;
            }
        }

        for (auto& [id, obj] : *activeGameObjects) {
            if (obj.texture) {
                obj.textureDirty = false;
            }
        }
    }

    std::shared_ptr<GenTexture> AppCtrl::getCachedTexture(const std::string& path) {
        auto it = textureCache.find(path);
        if (it != textureCache.end()) {
            return it->second;
        }

        auto newTexture = std::make_shared<GenTexture>(genDevice, path);
        textureCache[path] = newTexture;
        return newTexture;
    }

    void AppCtrl::updateNodeColorAndTextureFromPlayer(
        GenGameObject& player,
        GenGameObject::Map& gameObjects,
        const std::shared_ptr<GenTexture>& redTexture,
        const std::shared_ptr<GenTexture>& orangeTexture,
        const std::shared_ptr<GenTexture>& greenTexture,
        int currentFrameIndex,
        float elapsedTime
    )
    {
        GLFWwindow* window = genWindow.getGLFWwindow();
        if (!player.soundDisc) return;

        // Simple key press detection
        const std::vector<int> moveKeys = { GLFW_KEY_A, GLFW_KEY_S, GLFW_KEY_D, GLFW_KEY_W };
        bool isMovingKeyPressed = false;
        for (int key : moveKeys) {
            if (glfwGetKey(window, key) == GLFW_PRESS) {
                isMovingKeyPressed = true;
                break;
            }
        }

        if (!isMovingKeyPressed) return; // no input, no sound

        // Check modifiers
        bool loud = glfwGetKey(window, GLFW_KEY_LEFT_SHIFT) == GLFW_PRESS;
        bool quiet = glfwGetKey(window, GLFW_KEY_LEFT_CONTROL) == GLFW_PRESS;

        NodeComponent::NodeColor targetColor;
        std::shared_ptr<GenTexture> selectedTexture;
        float soundMultiplier = 1.0f;

        if (loud) {
            targetColor = NodeComponent::NodeColor::Red;
            selectedTexture = redTexture;
            soundMultiplier = 2.0f;
        }
        else if (quiet) {
            targetColor = NodeComponent::NodeColor::Green;
            selectedTexture = greenTexture;
            soundMultiplier = 0.5f;
        }
        else {
            targetColor = NodeComponent::NodeColor::Orange;
            selectedTexture = orangeTexture;
            soundMultiplier = 1.0f;
        }

        GenGameObject* closestNode = nullptr;
        GenGameObject* secondClosestNode = nullptr;
        float closestDistSq = std::numeric_limits<float>::max();
        float secondClosestDistSq = std::numeric_limits<float>::max();

        glm::vec2 playerCenter = { player.transform.translation.x, player.transform.translation.z };

        for (auto& [id, obj] : gameObjects) {
            if (obj.type != ObjectType::Node || !obj.node || !obj.soundDisc) continue;

            glm::vec2 nodeCenter = { obj.transform.translation.x, obj.transform.translation.z };
            float distSq = glm::distance2(playerCenter, nodeCenter);

            float combinedRadius = player.soundDisc->radius + obj.soundDisc->radius;
            if (distSq <= combinedRadius * combinedRadius) {
                if (distSq < closestDistSq) {
                    // Update both closest and second-closest
                    secondClosestDistSq = closestDistSq;
                    secondClosestNode = closestNode;

                    closestDistSq = distSq;
                    closestNode = &obj;
                }
                else if (distSq < secondClosestDistSq) {
                    secondClosestDistSq = distSq;
                    secondClosestNode = &obj;
                }
            }
        }

        // Apply change only if the closest is 20% closer (in squared distance)
        
        if (closestNode &&
            (secondClosestNode == nullptr || closestDistSq <= 0.8f * secondClosestDistSq)) {
            auto cooldownIt = nodeCooldowns.find(closestNode->getId());

            if (cooldownIt != nodeCooldowns.end() && elapsedTime < cooldownIt->second) {
                // Node is still in cooldown
                return;
            }

            auto& nodeObj = *closestNode;
            auto& node = *nodeObj.node;

            if (node.color != targetColor) {
                node.color = targetColor;
                node.activated = true;
                node.selfPosition = nodeObj.transform.translation;

                if (node.lastColorApplied != targetColor) {
                    node.lastColorApplied = targetColor;



                    const int cooldownFrames = 6;
                    nodeCooldowns[nodeObj.getId()] = elapsedTime + 7.0f;  // start 7-second cooldown
                    auto lastChange = lastTextureChangeFrame.find(nodeObj.getId());
                    if (lastChange == lastTextureChangeFrame.end() ||
                        (currentFrameIndex - lastChange->second >= cooldownFrames &&
                            nodeObj.texture.get() != selectedTexture.get())) 
                    {

                        nodeObj.texture = selectedTexture;
                        nodeObj.textureDirty = true;
                        lastTextureChangeFrame[nodeObj.getId()] = currentFrameIndex;

                        std::cout << "[SOUND] Node " << nodeObj.getId()
                            << " changed to " << static_cast<int>(targetColor) << "\n";
                    }
                }
            }
        }

    }


    void AppCtrl::resetLevelTransforms(const std::unordered_map<std::string, TransformComponent>& initialTransforms) {
        for (auto& [id, obj] : *activeGameObjects) {
            if (!obj.tag.empty()) {
                auto it = initialTransforms.find(obj.tag);
                if (it != initialTransforms.end()) {
                    obj.transform = it->second;
                }
            }
        }
    }

}