#include "gen_cubemap.hpp"
#include <stb_image.h>
#include <stdexcept>
#include <cstring>

namespace gen {

    GenCubemap::GenCubemap(GenDevice& device, const std::vector<std::string>& faceFilepaths)
        : device_(device) {
        loadCubemapFaces(faceFilepaths);
        createCubemapImage();
        transitionImageLayouts();
        createImageView();
        createSampler();
        cleanupStaging();

        descriptorInfo_.sampler = cubemapSampler_;
        descriptorInfo_.imageView = cubemapImageView_;
        descriptorInfo_.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
    }

    GenCubemap::~GenCubemap() {
        vkDestroySampler(device_.device(), cubemapSampler_, nullptr);
        vkDestroyImageView(device_.device(), cubemapImageView_, nullptr);
        vkDestroyImage(device_.device(), cubemapImage_, nullptr);
        vkFreeMemory(device_.device(), cubemapImageMemory_, nullptr);
    }

    void GenCubemap::loadCubemapFaces(const std::vector<std::string>& paths) {
        if (paths.size() != 6) {
            throw std::runtime_error("Cubemap requires 6 face images");
        }

        int texChannels;
        std::vector<unsigned char*> facePixels(6);
        for (int i = 0; i < 6; i++) {
            facePixels[i] = stbi_load(paths[i].c_str(), reinterpret_cast<int*>(&width_), reinterpret_cast<int*>(&height_), &texChannels, STBI_rgb_alpha);
            if (!facePixels[i]) {
                throw std::runtime_error("Failed to load cubemap face: " + paths[i]);
            }
        }

        VkDeviceSize imageSize = width_ * height_ * 4;
        allPixels_.resize(imageSize * 6);
        for (int i = 0; i < 6; i++) {
            std::memcpy(allPixels_.data() + imageSize * i, facePixels[i], static_cast<size_t>(imageSize));
            stbi_image_free(facePixels[i]);
        }

        device_.createBuffer(
            allPixels_.size(),
            VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
            VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
            stagingBuffer_,
            stagingBufferMemory_);

        void* data;
        vkMapMemory(device_.device(), stagingBufferMemory_, 0, allPixels_.size(), 0, &data);
        std::memcpy(data, allPixels_.data(), allPixels_.size());
        vkUnmapMemory(device_.device(), stagingBufferMemory_);
    }

    void GenCubemap::createCubemapImage() {
        VkImageCreateInfo imageInfo{};
        imageInfo.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
        imageInfo.imageType = VK_IMAGE_TYPE_2D;
        imageInfo.extent.width = width_;
        imageInfo.extent.height = height_;
        imageInfo.extent.depth = 1;
        imageInfo.mipLevels = 1;
        imageInfo.arrayLayers = 6;
        imageInfo.format = VK_FORMAT_R8G8B8A8_SRGB;
        imageInfo.tiling = VK_IMAGE_TILING_OPTIMAL;
        imageInfo.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
        imageInfo.usage = VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT;
        imageInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
        imageInfo.samples = VK_SAMPLE_COUNT_1_BIT;
        imageInfo.flags = VK_IMAGE_CREATE_CUBE_COMPATIBLE_BIT;

        if (vkCreateImage(device_.device(), &imageInfo, nullptr, &cubemapImage_) != VK_SUCCESS) {
            throw std::runtime_error("Failed to create cubemap image!");
        }

        VkMemoryRequirements memRequirements;
        vkGetImageMemoryRequirements(device_.device(), cubemapImage_, &memRequirements);

        VkMemoryAllocateInfo allocInfo{};
        allocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
        allocInfo.allocationSize = memRequirements.size;
        allocInfo.memoryTypeIndex = device_.findMemoryType(memRequirements.memoryTypeBits, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);

        if (vkAllocateMemory(device_.device(), &allocInfo, nullptr, &cubemapImageMemory_) != VK_SUCCESS) {
            throw std::runtime_error("Failed to allocate cubemap memory!");
        }

        vkBindImageMemory(device_.device(), cubemapImage_, cubemapImageMemory_, 0);
    }

    void GenCubemap::transitionImageLayouts() {
        device_.transitionImageLayout(
            cubemapImage_,
            VK_FORMAT_R8G8B8A8_SRGB,
            VK_IMAGE_LAYOUT_UNDEFINED,
            VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
            6);

        device_.copyBufferToImage(
            stagingBuffer_,
            cubemapImage_,
            width_,
            height_,
            6);

        device_.transitionImageLayout(
            cubemapImage_,
            VK_FORMAT_R8G8B8A8_SRGB,
            VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
            VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL,
            6);
    }

    void GenCubemap::createImageView() {
        VkImageViewCreateInfo viewInfo{};
        viewInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
        viewInfo.image = cubemapImage_;
        viewInfo.viewType = VK_IMAGE_VIEW_TYPE_CUBE;
        viewInfo.format = VK_FORMAT_R8G8B8A8_SRGB;
        viewInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
        viewInfo.subresourceRange.baseMipLevel = 0;
        viewInfo.subresourceRange.levelCount = 1;
        viewInfo.subresourceRange.baseArrayLayer = 0;
        viewInfo.subresourceRange.layerCount = 6;

        if (vkCreateImageView(device_.device(), &viewInfo, nullptr, &cubemapImageView_) != VK_SUCCESS) {
            throw std::runtime_error("Failed to create cubemap image view!");
        }
    }

    void GenCubemap::createSampler() {
        VkSamplerCreateInfo samplerInfo{};
        samplerInfo.sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;
        samplerInfo.magFilter = VK_FILTER_LINEAR;
        samplerInfo.minFilter = VK_FILTER_LINEAR;
        samplerInfo.addressModeU = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;
        samplerInfo.addressModeV = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;
        samplerInfo.addressModeW = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;
        samplerInfo.anisotropyEnable = VK_FALSE;
        samplerInfo.maxAnisotropy = 1.0f;
        samplerInfo.borderColor = VK_BORDER_COLOR_INT_OPAQUE_BLACK;
        samplerInfo.unnormalizedCoordinates = VK_FALSE;
        samplerInfo.compareEnable = VK_FALSE;
        samplerInfo.mipmapMode = VK_SAMPLER_MIPMAP_MODE_LINEAR;

        if (vkCreateSampler(device_.device(), &samplerInfo, nullptr, &cubemapSampler_) != VK_SUCCESS) {
            throw std::runtime_error("Failed to create cubemap sampler!");
        }
    }

    void GenCubemap::cleanupStaging() {
        vkDestroyBuffer(device_.device(), stagingBuffer_, nullptr);
        vkFreeMemory(device_.device(), stagingBufferMemory_, nullptr);
    }

}