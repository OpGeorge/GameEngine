#pragma once

#include "gen_device.hpp"
#include "gen_game_object.hpp"
#include "gen_pipeline.hpp"
#include "gen_frame_info.hpp"
#include "gen_point_shadow_map.hpp"

#include <memory>
#include <vector>

namespace gen {

    class SimpleShadowRenderSystem {
    public:
        SimpleShadowRenderSystem(GenDevice& device, VkRenderPass shadowRenderPass);
        ~SimpleShadowRenderSystem() = default;

        SimpleShadowRenderSystem(const SimpleShadowRenderSystem&) = delete;
        SimpleShadowRenderSystem& operator=(const SimpleShadowRenderSystem&) = delete;

        void renderObjects(
            VkCommandBuffer commandBuffer,
            GenPointShadowMap& shadowMap,
            const GenGameObject::Map& objects
        );

    private:
        void createPipelineLayout();
        void createPipeline(VkRenderPass renderPass);

        GenDevice& genDevice;

        std::unique_ptr<GenPipeline> pipeline;
        VkPipelineLayout pipelineLayout;
    };

}
