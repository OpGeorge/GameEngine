#pragma once

#include "gen_device.hpp"
#include "gen_game_object.hpp"

#include "gen_renderer.hpp"
#include "gen_window.hpp"

#include "gen_descriptors.hpp"

#include <memory>
#include <vector>


namespace gen {


	class AppCtrl {

	public:
		static constexpr int WIDTH = 800;
		static constexpr int HEIGHT = 600;

		AppCtrl();
		~AppCtrl();

		AppCtrl(const AppCtrl&) = delete;
		AppCtrl &operator=(const AppCtrl&) = delete;

		void run();

	private:

		void loadGameObjects();
	
		GenWindow genWindow{ WIDTH,HEIGHT,"HELLO VULKAN ENGINE!" };

		GenDevice genDevice{ genWindow };

		GenRenderer genRenderer{ genWindow,genDevice };

		std::unique_ptr<GenDescriptorPool> globalPool{};

		GenGameObject::Map gameObjects;


	};
}