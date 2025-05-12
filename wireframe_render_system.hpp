#pragma once

#include "gen_camera.hpp"
#include "gen_device.hpp"
#include "gen_game_object.hpp"
#include "gen_pipeline.hpp"
#include "gen_frame_info.hpp"

#include <memory>
#include <vector>

namespace gen {

    class WireframeRenderSystem {
    public:
        WireframeRenderSystem(GenDevice& device, VkRenderPass renderPass, VkDescriptorSetLayout globalSetLayout);
        ~WireframeRenderSystem();

        WireframeRenderSystem(const WireframeRenderSystem&) = delete;
        WireframeRenderSystem& operator=(const WireframeRenderSystem&) = delete;

        void render(FrameInfo& frameInfo);

    private:
        void createPipelineLayout(VkDescriptorSetLayout globalSetLayout);
        void createPipeline(VkRenderPass renderPass);

        GenDevice& genDevice;
        std::unique_ptr<GenPipeline> wirePipeline;
        VkPipelineLayout pipelineLayout;
    };
}
