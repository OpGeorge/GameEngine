#include "gen_texture_swapper.hpp"

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

    void TextureSwapper::swapTexture(GenGameObject& obj, const std::string& newTexturePath, int frameIndex) {
        obj.texture = std::make_shared<GenTexture>(genDevice, newTexturePath);

        VkDescriptorBufferInfo uboInfo = uboBuffers[frameIndex]->descriptorInfo();
        VkDescriptorImageInfo imageInfo = obj.texture->descriptorInfo();

        int useTextureFlag = 1;
        textureToggleBuffers[obj.getId()][frameIndex]->writeToBuffer(&useTextureFlag);
        VkDescriptorBufferInfo flagInfo = textureToggleBuffers[obj.getId()][frameIndex]->descriptorInfo();

        VkDescriptorSet descriptorSet;
        auto writer = GenDescriptorWriter(globalSetLayout, globalPool)
            .writeBuffer(0, &uboInfo)
            .writeImage(1, &imageInfo)
            .writeBuffer(2, &flagInfo);

        if (!writer.build(descriptorSet)) {
            throw std::runtime_error("Failed to rebuild descriptor set for texture swap");
        }

        objectDescriptorSets[frameIndex][obj.getId()] = descriptorSet;
    }

}