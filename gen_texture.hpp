#pragma once

#include "gen_device.hpp"

#include <string>
#include <vulkan/vulkan.h>

namespace gen {

    class GenTexture {
    public:
        GenTexture(GenDevice& device, const std::string& filepath);
        ~GenTexture();

        GenTexture(const GenTexture&) = delete;
        GenTexture& operator=(const GenTexture&) = delete;

        VkDescriptorImageInfo descriptorInfo() const;

    private:
        void createTextureImage(const std::string& filepath);
        void createTextureImageView();
        void createTextureSampler();

        GenDevice& genDevice;

        VkImage textureImage = VK_NULL_HANDLE;
        VkDeviceMemory textureImageMemory = VK_NULL_HANDLE;
        VkImageView textureImageView = VK_NULL_HANDLE;
        VkSampler textureSampler = VK_NULL_HANDLE;
    };

}  // namespace gen
