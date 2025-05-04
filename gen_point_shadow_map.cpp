#include "gen_point_shadow_map.hpp"
#include "gen_utils.hpp"

#include <array>
#include <stdexcept>
#include <glm/gtc/matrix_transform.hpp>

namespace gen {

    GenPointShadowMap::GenPointShadowMap(GenDevice& device, glm::vec3 lightPosition)
        : genDevice{ device }, lightPos{ lightPosition } {
        createDepthCubeMap();
        createRenderPass();
        createFramebuffers();

        projection = glm::perspective(glm::radians(90.0f), 1.0f, nearPlane, farPlane);
        projection[1][1] *= -1;  // Invert Y for Vulkan

        viewMatrices = {
            glm::lookAt(lightPos, lightPos + glm::vec3(1, 0, 0), glm::vec3(0, -1, 0)),  // +X
            glm::lookAt(lightPos, lightPos + glm::vec3(-1, 0, 0), glm::vec3(0, -1, 0)), // -X
            glm::lookAt(lightPos, lightPos + glm::vec3(0, 1, 0), glm::vec3(0, 0, 1)),   // +Y
            glm::lookAt(lightPos, lightPos + glm::vec3(0, -1, 0), glm::vec3(0, 0, -1)), // -Y
            glm::lookAt(lightPos, lightPos + glm::vec3(0, 0, 1), glm::vec3(0, -1, 0)),  // +Z
            glm::lookAt(lightPos, lightPos + glm::vec3(0, 0, -1), glm::vec3(0, -1, 0))  // -Z
        };
    }

    GenPointShadowMap::~GenPointShadowMap() {

        for (auto view : faceImageViews) {
            if (view != VK_NULL_HANDLE) {
                vkDestroyImageView(genDevice.device(), view, nullptr);
            }
        }

        vkDestroySampler(genDevice.device(), shadowSampler, nullptr);
        vkDestroyImageView(genDevice.device(), cubeMapImageView, nullptr);
        vkDestroyImage(genDevice.device(), cubeMapImage, nullptr);
        vkFreeMemory(genDevice.device(), cubeMapMemory, nullptr);

        for (auto fb : framebuffers) {
            vkDestroyFramebuffer(genDevice.device(), fb, nullptr);
        }

        vkDestroyRenderPass(genDevice.device(), renderPass, nullptr);
    }

    void GenPointShadowMap::createDepthCubeMap() {
        VkFormat depthFormat = VK_FORMAT_D32_SFLOAT;

        VkImageCreateInfo imageInfo{};
        imageInfo.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
        imageInfo.imageType = VK_IMAGE_TYPE_2D;
        imageInfo.extent.width = SHADOW_MAP_RESOLUTION;
        imageInfo.extent.height = SHADOW_MAP_RESOLUTION;
        imageInfo.extent.depth = 1;
        imageInfo.mipLevels = 1;
        imageInfo.arrayLayers = 6;
        imageInfo.format = depthFormat;
        imageInfo.tiling = VK_IMAGE_TILING_OPTIMAL;
        imageInfo.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
        imageInfo.usage = VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT | VK_IMAGE_USAGE_SAMPLED_BIT;
        imageInfo.samples = VK_SAMPLE_COUNT_1_BIT;
        imageInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
        imageInfo.flags = VK_IMAGE_CREATE_CUBE_COMPATIBLE_BIT;

        genDevice.createImageWithInfo(imageInfo, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, cubeMapImage, cubeMapMemory);

        VkImageViewCreateInfo viewInfo{};
        viewInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
        viewInfo.image = cubeMapImage;
        viewInfo.viewType = VK_IMAGE_VIEW_TYPE_CUBE;
        viewInfo.format = depthFormat;
        viewInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_DEPTH_BIT;
        viewInfo.subresourceRange.baseMipLevel = 0;
        viewInfo.subresourceRange.levelCount = 1;
        viewInfo.subresourceRange.baseArrayLayer = 0;
        viewInfo.subresourceRange.layerCount = 6;

        if (vkCreateImageView(genDevice.device(), &viewInfo, nullptr, &cubeMapImageView) != VK_SUCCESS) {
            throw std::runtime_error("failed to create cube map image view!");
        }

        VkSamplerCreateInfo samplerInfo{};
        samplerInfo.sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;
        samplerInfo.magFilter = VK_FILTER_LINEAR;
        samplerInfo.minFilter = VK_FILTER_LINEAR;
        samplerInfo.mipmapMode = VK_SAMPLER_MIPMAP_MODE_LINEAR;
        samplerInfo.addressModeU = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;
        samplerInfo.addressModeV = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;
        samplerInfo.addressModeW = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;
        samplerInfo.borderColor = VK_BORDER_COLOR_FLOAT_OPAQUE_WHITE;
        samplerInfo.compareEnable = VK_TRUE;
        samplerInfo.compareOp = VK_COMPARE_OP_LESS;
        samplerInfo.minLod = 0.0f;
        samplerInfo.maxLod = 1.0f;

        if (vkCreateSampler(genDevice.device(), &samplerInfo, nullptr, &shadowSampler) != VK_SUCCESS) {
            throw std::runtime_error("failed to create shadow map sampler!");
        }
    }

    void GenPointShadowMap::createRenderPass() {
        VkAttachmentDescription depthAttachment{};
        depthAttachment.format = VK_FORMAT_D32_SFLOAT;
        depthAttachment.samples = VK_SAMPLE_COUNT_1_BIT;
        depthAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
        depthAttachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
        depthAttachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
        depthAttachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
        depthAttachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
        depthAttachment.finalLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;

        VkAttachmentReference depthAttachmentRef{};
        depthAttachmentRef.attachment = 0;
        depthAttachmentRef.layout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

        VkSubpassDescription subpass{};
        subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
        subpass.colorAttachmentCount = 0;
        subpass.pDepthStencilAttachment = &depthAttachmentRef;

        VkRenderPassCreateInfo renderPassInfo{};
        renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
        renderPassInfo.attachmentCount = 1;
        renderPassInfo.pAttachments = &depthAttachment;
        renderPassInfo.subpassCount = 1;
        renderPassInfo.pSubpasses = &subpass;

        if (vkCreateRenderPass(genDevice.device(), &renderPassInfo, nullptr, &renderPass) != VK_SUCCESS) {
            throw std::runtime_error("failed to create shadow render pass!");
        }
    }

    void GenPointShadowMap::createFramebuffers() {
        for (int i = 0; i < 6; ++i) {
            VkImageViewCreateInfo viewInfo{};
            viewInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
            viewInfo.image = cubeMapImage;
            viewInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
            viewInfo.format = VK_FORMAT_D32_SFLOAT;
            viewInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_DEPTH_BIT;
            viewInfo.subresourceRange.baseMipLevel = 0;
            viewInfo.subresourceRange.levelCount = 1;
            viewInfo.subresourceRange.baseArrayLayer = i;
            viewInfo.subresourceRange.layerCount = 1;

            VkImageView faceView;
            if (vkCreateImageView(genDevice.device(), &viewInfo, nullptr, &faceView) != VK_SUCCESS) {
                throw std::runtime_error("failed to create face view for shadow cube map!");
            }

            VkFramebufferCreateInfo fbInfo{};
            fbInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
            fbInfo.renderPass = renderPass;
            fbInfo.attachmentCount = 1;
            fbInfo.pAttachments = &faceView;
            fbInfo.width = SHADOW_MAP_RESOLUTION;
            fbInfo.height = SHADOW_MAP_RESOLUTION;
            fbInfo.layers = 1;

            if (vkCreateFramebuffer(genDevice.device(), &fbInfo, nullptr, &framebuffers[i]) != VK_SUCCESS) {
                throw std::runtime_error("failed to create shadow framebuffer!");
            }

            faceImageViews[i] = faceView;
        }
    }

    void GenPointShadowMap::beginFaceRenderPass(VkCommandBuffer cmd, int faceIndex) {
        VkClearValue clearValue{};
        clearValue.depthStencil = { 1.0f, 0 };

        VkRenderPassBeginInfo renderPassInfo{};
        renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
        renderPassInfo.renderPass = renderPass;
        renderPassInfo.framebuffer = framebuffers[faceIndex];
        renderPassInfo.renderArea.offset = { 0, 0 };
        renderPassInfo.renderArea.extent = { SHADOW_MAP_RESOLUTION, SHADOW_MAP_RESOLUTION };
        renderPassInfo.clearValueCount = 1;
        renderPassInfo.pClearValues = &clearValue;

        vkCmdBeginRenderPass(cmd, &renderPassInfo, VK_SUBPASS_CONTENTS_INLINE);
    }

    void GenPointShadowMap::endRenderPass(VkCommandBuffer cmd) {
        vkCmdEndRenderPass(cmd);
    }

    glm::mat4 GenPointShadowMap::getLightSpaceMatrix(int faceIndex) const {
        return projection * viewMatrices[faceIndex];
    }

    VkDescriptorImageInfo GenPointShadowMap::descriptorInfo() const {
        VkDescriptorImageInfo info{};
        info.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
        info.imageView = cubeMapImageView;
        info.sampler = shadowSampler;
        return info;
    }

}
