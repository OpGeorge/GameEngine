#include "gen_texture_swapper.hpp"

#include <stdexcept>
#include <cstdlib>
#include <iostream>
#include <stdexcept>

namespace gen {

    TextureSwapper::TextureSwapper(
        GenDevice& device,
        GenDescriptorSetLayout& setLayout,
        GenDescriptorPool& pool,
        std::vector<std::unique_ptr<GenBuffer>>& uboBuffers,
        std::unordered_map<GenGameObject::id_t, std::vector<std::unique_ptr<GenBuffer>>>& textureToggleBuffers,
        std::array<std::unordered_map<GenGameObject::id_t, VkDescriptorSet>, GenSwapChain::MAX_FRAMES_IN_FLIGHT>& objectDescriptorSets)
        : genDevice{ device },
        globalSetLayout{ setLayout },
        globalPool{ pool },
        uboBuffers{ uboBuffers },
        textureToggleBuffers{ textureToggleBuffers },
        objectDescriptorSets{ objectDescriptorSets }
    {
    }

    void TextureSwapper::swapTexture(
        GenGameObject& obj,
        std::shared_ptr<GenTexture> newTexture,
        int frameIndex)
    {
        obj.texture = newTexture;

        VkDescriptorBufferInfo uboInfo = uboBuffers[frameIndex]->descriptorInfo();
        VkDescriptorImageInfo imageInfo = newTexture->descriptorInfo();

        int useTextureFlag = 1;
        textureToggleBuffers[obj.getId()][frameIndex]->writeToBuffer(&useTextureFlag);
        VkDescriptorBufferInfo flagInfo = textureToggleBuffers[obj.getId()][frameIndex]->descriptorInfo();

        VkDescriptorSet descriptorSet;
        auto writer = GenDescriptorWriter(globalSetLayout, globalPool)
            .writeBuffer(0, &uboInfo)
            .writeImage(1, &imageInfo)
            .writeBuffer(2, &flagInfo);

        if (!writer.build(descriptorSet)) {
            std::cerr << "[TEXTURE SWAP FAIL] Descriptor set allocation failed for object "
                << obj.getId() << ". Will retry.\n";
            return;
        }

        objectDescriptorSets[frameIndex][obj.getId()] = descriptorSet;
        obj.textureDirty = false; // Mark as clean
    }

}