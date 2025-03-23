#pragma once

#include "gen_camera.hpp"
#include <vulkan/vulkan.h>


namespace gen {

	struct FrameInfo {
		int frameIndex;
		float frameTime;
		VkCommandBuffer commandBuffer;
		GenCamera& camera;
		VkDescriptorSet globalDescriptorSet; 
	};
	


}