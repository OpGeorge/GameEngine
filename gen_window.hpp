#pragma once

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>
#include <string>
namespace gen {
	
	class GenWindow {

	public:
		GenWindow(int w, int h, std::string name);
		~GenWindow();

		GenWindow(const GenWindow&) = delete;
		GenWindow &operator=(const GenWindow&) = delete;


		bool shouldClose() { return glfwWindowShouldClose(window); }

		VkExtent2D getExtent() { return { static_cast<uint32_t> (width), static_cast<uint32_t> (height) }; }

		bool wasWindowResised() { return framebufferResized; }

		void resetWindowResisezedFlag() { framebufferResized = false; }

		void createWindowSurface(VkInstance instance, VkSurfaceKHR* surface);

		GLFWwindow* getGLFWwindow() const { return window; };

	private:

		static void framebufferResizeCallback(GLFWwindow * window, int width, int height);

		void inintWindow();

		int width;
		int height;
		bool framebufferResized = false;
		
		std::string windowName;

		GLFWwindow* window;


	};
}