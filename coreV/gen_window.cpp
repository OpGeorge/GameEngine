#include "gen_window.hpp"

#include <stdexcept>

namespace gen {

	GenWindow::GenWindow(int w, int h, std::string name) : width{ w }, height{ h }, windowName{ name } {
		inintWindow();
	}

	GenWindow::~GenWindow() {
		glfwDestroyWindow(window);
		glfwTerminate();

	}

	void GenWindow::inintWindow() {
		glfwInit();
		glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
		glfwWindowHint(GLFW_RESIZABLE, GLFW_TRUE);

		window = glfwCreateWindow(width, height, windowName.c_str(), nullptr, nullptr);
		glfwSetWindowUserPointer(window, this);
		glfwSetFramebufferSizeCallback(window, framebufferResizeCallback);


	}

	void GenWindow::createWindowSurface(VkInstance instance, VkSurfaceKHR* surface) {
	
		if (glfwCreateWindowSurface(instance, window, nullptr, surface) != VK_SUCCESS) {
			
			throw std::runtime_error("failed to create window surface");
			
		}
	}

	void GenWindow::framebufferResizeCallback(GLFWwindow* window, int width, int height) {
	
		auto genWindow = reinterpret_cast<GenWindow*>(glfwGetWindowUserPointer(window));
		genWindow->framebufferResized = true;
		genWindow->width = width;
		genWindow->height = height;

	}


}