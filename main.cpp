#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include <glm/vec4.hpp>
#include <glm/mat4x4.hpp>
#include <vector>

#include <iostream>
#include <stdexcept>
#include <cstdlib>

const uint32_t WIDTH = 800;
const uint32_t HEIGHT = 600;

const std::vector<const char*> validationLayers = {
	"VK_LAYER_KHRONOS_validation"
};

#ifdef NDEBUG
 const bool enableValidationLayers = false;
#else
 const bool enableValidationLayers = true;
#endif

	

class TriangleApplication{
	public:
		void run(){
			initWindow();
			initVulkan();
			mainloop();
			cleanup();
		}
	private:
		GLFWwindow* window;
		VkInstance instance;
		
		void createInstance(){
		VkApplicationInfo appInfo{};
		appInfo.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
		appInfo.pApplicationName = "Triangle Application";
		appInfo.applicationVersion = VK_MAKE_VERSION(0,0,1);
		appInfo.pEngineName = "none";
		appInfo.engineVersion = VK_MAKE_VERSION(1,0,0);
		appInfo.apiVersion = VK_API_VERSION_1_0;
		VkInstanceCreateInfo createInfo{};
		createInfo.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
		createInfo.pApplicationInfo = &appInfo;
		
		uint32_t glfwExtensionCount = 0;
		const char** glfwExtensions;
		
		glfwExtensions = glfwGetRequiredInstanceExtensions(&glfwExtensionCount);

		createInfo.enabledExtensionCount = glfwExtensionCount;
		createInfo.ppEnabledExtensionNames = glfwExtensions;
		uint32_t vkExtensionCount = 0;
		vkEnumerateInstanceExtensionProperties(nullptr, &vkExtensionCount, nullptr);
		std::vector<VkExtensionProperties> extensions(vkExtensionCount);
		vkEnumerateInstanceExtensionProperties(nullptr, &vkExtensionCount, extensions.data());
		std::cout << "Avaliable extensions:\n";
		for(const auto& extension:extensions){
			std::cout << "/t" << extension.extensionName<<"\n";
		}
		createInfo.enabledLayerCount = 0;
		if(vkCreateInstance(&createInfo, nullptr, &instance) != VK_SUCCESS){
				throw std::runtime_error("Failed to create Vulkan instance!");
			}
		}

		void initWindow(){
		glfwInit();
		glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
		glfwWindowHint(GLFW_RESIZABLE, GLFW_FALSE);
		window = glfwCreateWindow(WIDTH, HEIGHT, "Vulcan", nullptr, nullptr);
		}

		void initVulkan(){
			createInstance();
		};

		void mainloop(){
			while(!glfwWindowShouldClose(window)){
				glfwPollEvents();
			}
		};

		void cleanup(){
			vkDestroyInstance(instance, nullptr);
			glfwDestroyWindow(window);
			glfwTerminate();
		};	
};

int main() {
	TriangleApplication app;

	try{
		app.run();
	} catch(const std::exception& e){
		std::cerr<<e.what()<<std::endl;
		return EXIT_FAILURE;
	}

	return EXIT_SUCCESS;
}
