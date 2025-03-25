#pragma once
#pragma once

#include "gen_camera.hpp"
#include "gen_device.hpp"
#include "gen_game_object.hpp"
#include "gen_pipeline.hpp"
#include "gen_frame_info.hpp"







#include <memory>
#include <vector>


namespace gen {


	class PointLightSystem {

	public:


		PointLightSystem(GenDevice& device, VkRenderPass renderPass, VkDescriptorSetLayout globalSetLayout);
		~PointLightSystem();

		PointLightSystem(const PointLightSystem&) = delete;
		PointLightSystem& operator=(const PointLightSystem&) = delete;


		void update(FrameInfo& frameInfo, GlobalUbo& ubo);
		void render(FrameInfo& frameInfo);

	private:


		void createPipelineLayot(VkDescriptorSetLayout globalSetLayout);
		void createPipeline(VkRenderPass renderPass);







		GenDevice& genDevice;



		std::unique_ptr<GenPipeline> genPipeline;

		VkPipelineLayout pipelineLayout;





	};
}