#pragma once

#include "coreV/gen_device.hpp"
#include <vulkan/vulkan.h>
#include <string>
#include <vector>

 namespace gen {

    class GenCubemap {
    public:
        GenCubemap(GenDevice& device, const std::vector<std::string>& faceFilepaths);
        ~GenCubemap();

        GenCubemap(const GenCubemap&) = delete;
        GenCubemap& operator=(const GenCubemap&) = delete;

        VkDescriptorImageInfo descriptorInfo() const { return descriptorInfo_; }

    private:
        void loadCubemapFaces(const std::vector<std::string>& paths);
        void createCubemapImage();
        void createImageView();
        void createSampler();
        void transitionImageLayouts();
        void cleanupStaging();

        GenDevice& device_;

        VkImage cubemapImage_{ VK_NULL_HANDLE };
        VkDeviceMemory cubemapImageMemory_{ VK_NULL_HANDLE };
        VkImageView cubemapImageView_{ VK_NULL_HANDLE };
        VkSampler cubemapSampler_{ VK_NULL_HANDLE };
        VkDescriptorImageInfo descriptorInfo_{};

        VkBuffer stagingBuffer_{ VK_NULL_HANDLE };
        VkDeviceMemory stagingBufferMemory_{ VK_NULL_HANDLE };

        std::vector<unsigned char> allPixels_;
        uint32_t width_{ 0 }, height_{ 0 };
    };
}

