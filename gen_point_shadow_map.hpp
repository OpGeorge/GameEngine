#pragma once

#include "gen_device.hpp"
#include <vulkan/vulkan.h>
#include <glm/glm.hpp>
#include <array>

namespace gen {

    class GenPointShadowMap {
    public:
        static constexpr uint32_t SHADOW_MAP_RESOLUTION = 1024;

        GenPointShadowMap(GenDevice& device, glm::vec3 lightPosition);
        ~GenPointShadowMap();

        GenPointShadowMap(const GenPointShadowMap&) = delete;
        GenPointShadowMap& operator=(const GenPointShadowMap&) = delete;

        // Rendering interface
        void beginFaceRenderPass(VkCommandBuffer commandBuffer, int faceIndex);
        void endRenderPass(VkCommandBuffer commandBuffer);

        // Accessors
        VkRenderPass getRenderPass() const { return renderPass; }
        VkSampler getShadowSampler() const { return shadowSampler; }
        VkImageView getCubeMapView() const { return cubeMapImageView; }
        VkDescriptorImageInfo descriptorInfo() const;

        // Matrix per cube face
        glm::mat4 getLightSpaceMatrix(int faceIndex) const;

    private:
        void createDepthCubeMap();
        void createRenderPass();
        void createFramebuffers();

        GenDevice& genDevice;

        

        glm::vec3 lightPos;
        float nearPlane = 0.1f;
        float farPlane = 25.0f;

        VkImage cubeMapImage;
        VkDeviceMemory cubeMapMemory;
        VkImageView cubeMapImageView;
        VkSampler shadowSampler;

        VkRenderPass renderPass;
        std::array<VkFramebuffer, 6> framebuffers;

        glm::mat4 projection;
        std::array<glm::mat4, 6> viewMatrices;

        std::array<VkImageView, 6> faceImageViews{};

    };

}
