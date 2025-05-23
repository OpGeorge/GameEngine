#pragma once

#include "../gen_game_object.hpp"
#include "gen_descriptors.hpp"
#include "gen_buffer.hpp"
#include "gen_texture.hpp"
#include "gen_swap_chain.hpp"

#include <unordered_map>
#include <array>
#include <vector>
#include <memory>
#include <string>

namespace gen {

    class TextureSwapper {
    public:
        TextureSwapper(
            GenDevice& device,
            GenDescriptorSetLayout& setLayout,
            GenDescriptorPool& pool,
            std::vector<std::unique_ptr<GenBuffer>>& uboBuffers,
            std::unordered_map<GenGameObject::id_t, std::vector<std::unique_ptr<GenBuffer>>>& textureToggleBuffers,
            std::array<std::unordered_map<GenGameObject::id_t, VkDescriptorSet>, GenSwapChain::MAX_FRAMES_IN_FLIGHT>& objectDescriptorSets);

        void swapTexture(
            GenGameObject& obj,
            std::shared_ptr<GenTexture> newTexture,
            int frameIndex);

    private:
        GenDevice& genDevice;
        GenDescriptorSetLayout& globalSetLayout;
        GenDescriptorPool& globalPool;
        std::vector<std::unique_ptr<GenBuffer>>& uboBuffers;
        std::unordered_map<GenGameObject::id_t, std::vector<std::unique_ptr<GenBuffer>>>& textureToggleBuffers;
        std::array<std::unordered_map<GenGameObject::id_t, VkDescriptorSet>, GenSwapChain::MAX_FRAMES_IN_FLIGHT>& objectDescriptorSets;
    };

}