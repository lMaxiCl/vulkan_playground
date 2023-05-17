#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include <glm/vec4.hpp>
#include <glm/mat4x4.hpp>
#include <vector>
#include <cstring>
#include <iostream>
#include <stdexcept>
#include <cstdlib>
#include <optional>
#include <set>

const uint32_t WIDTH = 800;
const uint32_t HEIGHT = 600;

const std::vector<const char*> validationLayers = {
	"VK_LAYER_KHRONOS_validation"
};

const std::vector<const char*> deviceExtensions = {
	VK_KHR_SWAPCHAIN_EXTENSION_NAME
};

#ifdef NDEBUG
	const bool enableValidationLayers = false;
#else
	const bool enableValidationLayers = true;
#endif

struct QueueFamilyIndeces{
	std::optional<uint32_t> graphicsFamily;
	std::optional<uint32_t> presentFamily;

	bool isComplete(){
		return graphicsFamily.has_value() && presentFamily.has_value();
	}
};

bool checkValidationLayersSupport(){
	uint32_t layerCount;

	vkEnumerateInstanceLayerProperties(&layerCount, nullptr);
	std::vector<VkLayerProperties> availableLayers(layerCount);
	vkEnumerateInstanceLayerProperties(&layerCount, availableLayers.data());
	
	for(const char* layerName:validationLayers){
		bool layerFound = false;

		for(const auto& layerProperties:availableLayers){
			if(strcmp(layerName, layerProperties.layerName)==0){
				layerFound = true;
				break;
			}
		}

		if(!layerFound){
			return false;
		}
	}

	return true;
}	


class TriangleApplication{
	public:
		void run(){
			initWindow();
			initVulkan();
			mainloop();
			cleanup();
		}
	private:
		//for storing window where content will be displayed
		GLFWwindow* window;
		//for storing vulkan instance
		VkInstance instance;
		//for storing extension for messenger
		VkDebugUtilsMessengerEXT debugMessenger;
		//for storing physical device
		VkPhysicalDevice physicalDevice = VK_NULL_HANDLE;
		//for storing logical device which binds to physical device
		VkDevice device;
		//vulkan queue
		VkQueue graphicsQueue;
		//object for linking window to vulkan
		VkSurfaceKHR surface;
		//vkQueue for presentation device queue handling
		VkQueue presentQueue;

		//general
		//glfw window initialization
		void initWindow(){
			glfwInit();
			glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
			glfwWindowHint(GLFW_RESIZABLE, GLFW_FALSE);
			window = glfwCreateWindow(WIDTH, HEIGHT, "Vulcan", nullptr, nullptr);
		}

		//vulkan initialization
		void initVulkan(){
			createInstance();
			setupDebugMessengerEXT();
			createSurface();
			pickPhysicalDevice();
			createLogicalDevice();
		};

		//main loop of an application
		void mainloop(){
			while(!glfwWindowShouldClose(window)){
				glfwPollEvents();
			}
		};

		//cleanup (memory reallocation etc.)
		void cleanup(){
			vkDestroyDevice(device, nullptr);
			if(enableValidationLayers){
				DestroyDebugMessengerEXT(instance, debugMessenger, nullptr);
				}
			vkDestroySurfaceKHR(instance, surface, nullptr);
			vkDestroyInstance(instance, nullptr);
			glfwDestroyWindow(window);
			glfwTerminate();
		};

		//debuging messenger extension setup
		void setupDebugMessengerEXT(){
			if(!enableValidationLayers) return;

			VkDebugUtilsMessengerCreateInfoEXT createInfo{};
			populateDebugMessengerCreateInfo(createInfo);

			if(CreateDebugMessengerEXT(instance, &createInfo, nullptr, &debugMessenger) != VK_SUCCESS){
				throw std::runtime_error("Failed to set up debug messenger!");
			}
		}

		//debugMessenger
		//creating createIngo for debug messenger initialization
		void populateDebugMessengerCreateInfo(VkDebugUtilsMessengerCreateInfoEXT& createInfo){
			createInfo = {};
			createInfo.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT;
			createInfo.messageSeverity = 
				VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT |
				VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT |
				VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT;
			createInfo.messageType = 
				VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT |
				VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT |
				VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT;
			createInfo.pfnUserCallback = debugCallback;
			createInfo.pUserData = nullptr;
			createInfo.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT;
		}

		//destroing debug messenger
		void DestroyDebugMessengerEXT(VkInstance instance,
				VkDebugUtilsMessengerEXT debugMessenger,
				const VkAllocationCallbacks* pAllocator){
					if(!enableValidationLayers) return;

					auto func = (PFN_vkDestroyDebugUtilsMessengerEXT)
						vkGetInstanceProcAddr(instance,	
								"vkDestroyDebugUtilsMessengerEXT");

					if(func != nullptr){
						func(instance, debugMessenger, pAllocator);
					}
				}
		
		//creating debug messenger
		VkResult CreateDebugMessengerEXT(
				VkInstance instance,
				const VkDebugUtilsMessengerCreateInfoEXT* pCreateInfo,
				const VkAllocationCallbacks* pAllocator,
				VkDebugUtilsMessengerEXT* pDebugMessenger){

					auto func = (PFN_vkCreateDebugUtilsMessengerEXT)
						vkGetInstanceProcAddr(instance,
								"vkCreateDebugUtilsMessengerEXT");
					if(func != nullptr){
						return func(instance, pCreateInfo, pAllocator, pDebugMessenger);
					}else{
						return VK_ERROR_EXTENSION_NOT_PRESENT;
					}
				}

		//window
		//debug callback. for proper debug messenger work
		static VKAPI_ATTR VkBool32 VKAPI_CALL debugCallback(
			VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity,
			VkDebugUtilsMessageTypeFlagsEXT messageType,
			const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData,
			void* pUserData){
				std::cerr<<"Validation layer: "<<pCallbackData->pMessage<<std::endl;
				return VK_FALSE;
		}
	
		//obtainig extensions for glfw window
		std::vector<const char*> getRequiredExtensions(){
			uint32_t glfwExtensionCount = 0;
			const char** glfwExtensions;
			glfwExtensions = glfwGetRequiredInstanceExtensions(&glfwExtensionCount);
			std::vector<const char*> extensions(glfwExtensions, glfwExtensions + glfwExtensionCount);

			if(enableValidationLayers){
				extensions.push_back(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);
			}

			return extensions;
		}	

		void createSurface(){
			if(glfwCreateWindowSurface(instance, window, nullptr, &surface)!=VK_SUCCESS){
				throw std::runtime_error("Failed to create window surface!");
			}
		}

		//vulkan itself
		// creation of vulkan instance
		void createInstance(){
			if(enableValidationLayers && !checkValidationLayersSupport()){
				throw std::runtime_error("ValidationLayers requested, but not available! Aborting!");	
			}

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
			
			VkDebugUtilsMessengerCreateInfoEXT debugCreateInfo{};


			if(enableValidationLayers){
				createInfo.enabledLayerCount = static_cast<uint32_t>(validationLayers.size());
				createInfo.ppEnabledLayerNames = validationLayers.data();
				populateDebugMessengerCreateInfo(debugCreateInfo);
				createInfo.pNext = (VkDebugUtilsMessengerCreateInfoEXT*) &debugCreateInfo;
			}else{
				createInfo.enabledLayerCount=0;
				createInfo.pNext = nullptr;
			}


			if(enableValidationLayers){
				createInfo.enabledLayerCount = static_cast<uint32_t>(validationLayers.size());
				createInfo.ppEnabledLayerNames = validationLayers.data();
			}else{
				createInfo.enabledLayerCount=0;
			}

			auto extensions = getRequiredExtensions();
			createInfo.enabledExtensionCount = static_cast<uint32_t>(extensions.size());
			createInfo.ppEnabledExtensionNames = extensions.data();
			createInfo.enabledLayerCount = 0;

			uint32_t vkExtensionCount = 0;
			
			vkEnumerateInstanceExtensionProperties(nullptr, &vkExtensionCount, nullptr);
			std::vector<VkExtensionProperties> vkExtensions(vkExtensionCount);
			vkEnumerateInstanceExtensionProperties(nullptr, &vkExtensionCount, vkExtensions.data());
			

			if(vkCreateInstance(&createInfo, nullptr, &instance) != VK_SUCCESS){
				throw std::runtime_error("Failed to create Vulkan instance!");
			}
		};



		// choosing physical device with vulkan support and highest rating
		void pickPhysicalDevice(){
			uint32_t deviceCount = 0;	
			vkEnumeratePhysicalDevices(instance, &deviceCount, nullptr);
			if(deviceCount == 0){
				throw std::runtime_error("Failed to find GPUs with Vulkan support!");
			}
			std::vector<VkPhysicalDevice> devices(deviceCount);



			vkEnumeratePhysicalDevices(instance, &deviceCount, devices.data());
			uint32_t currentDeviceRating = 0;	
			for(const auto& device:devices){
				if(isDeviceSuitable(device) && rateDeviceSuitability(device) > currentDeviceRating)	{
					physicalDevice = device;
					currentDeviceRating = rateDeviceSuitability(device);
					break;
				}
			}
			
			if(physicalDevice == VK_NULL_HANDLE){
				throw std::runtime_error("Failed to find a suitable GPU!");
			}
		};
		
		//checking device queue families
		QueueFamilyIndeces findQueueFamilies (VkPhysicalDevice device){
			//place to find queue families
			QueueFamilyIndeces indeces;
			
			uint32_t queueFamilyCount = 0;
			vkGetPhysicalDeviceQueueFamilyProperties(device, &queueFamilyCount, nullptr);
			
			std::vector<VkQueueFamilyProperties> queueFamilies(queueFamilyCount);
			
			vkGetPhysicalDeviceQueueFamilyProperties(device, &queueFamilyCount, queueFamilies.data());
			
			int i = 0;
			for(const auto& queueFamily:queueFamilies){
				VkBool32 presentSupport=false;
				vkGetPhysicalDeviceSurfaceSupportKHR(device, i, surface, &presentSupport);
				if(queueFamily.queueFlags & VK_QUEUE_GRAPHICS_BIT){
					indeces.graphicsFamily = i;

				}
				if(presentSupport){
					indeces.presentFamily = i;
				}


				i++;
				if(indeces.isComplete()){
					break;
				}
			}

			return indeces;
		}
		
		//help function to rate which device is better
		int rateDeviceSuitability(VkPhysicalDevice device){
			uint32_t deviceRating =0;

			VkPhysicalDeviceProperties deviceProperties;
			VkPhysicalDeviceFeatures deviceFeatures;

			if(deviceProperties.deviceType == VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU){
				deviceRating += 1000;
			}

			deviceRating += deviceProperties.limits.maxImageDimension2D;

			if(!deviceFeatures.geometryShader){
				return 0;
			}

			return deviceRating;
		}

		//checking if device is suitable for our needs
		bool isDeviceSuitable(VkPhysicalDevice device){
			QueueFamilyIndeces indeces = findQueueFamilies(device);
			bool extensionsSupported = checkDeviceExtensionsSupport(device);
			return indeces.isComplete() && extensionsSupported;
		}

		bool checkDeviceExtensionsSupport(VkPhysicalDevice device){
			uint32_t extensionCount;
			vkEnumerateDeviceExtensionProperties(device, nullptr, &extensionCount, nullptr);
			std::vector<VkExtensionProperties> availableExtensions(extensionCount);
			vkEnumerateDeviceExtensionProperties(device, nullptr, &extensionCount, availableExtensions.data());

			std::set<std::string> requiredExtensions(deviceExtensions.begin(), deviceExtensions.end());

			for(const auto& extension:availableExtensions){
				requiredExtensions.erase(extension.extensionName);
			}

			return requiredExtensions.empty();
		}
		
		//creating logical device
		void createLogicalDevice(){
			QueueFamilyIndeces indeces = findQueueFamilies(physicalDevice);
			
			std::vector<VkDeviceQueueCreateInfo> queueCreateInfos;
			std::set<uint32_t> uniqueQueueFamilies = {indeces.graphicsFamily.value(), indeces.presentFamily.value()};

			float queuePriority = 1.0f;

			for (uint32_t queueFamily : uniqueQueueFamilies) {
				VkDeviceQueueCreateInfo queueCreateInfo{};
				queueCreateInfo.queueFamilyIndex = queueFamily;
				queueCreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
				queueCreateInfo.queueFamilyIndex = indeces.graphicsFamily.value();
				queueCreateInfo.queueCount = 1;
				queueCreateInfo.pQueuePriorities = &queuePriority;
				queueCreateInfos.push_back(queueCreateInfo);
			}
			VkPhysicalDeviceFeatures deviceFeatures{};
			
			VkDeviceCreateInfo createInfo{};
			createInfo.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
			createInfo.pQueueCreateInfos = queueCreateInfos.data();
			createInfo.queueCreateInfoCount = static_cast<uint32_t>(queueCreateInfos.size());
			createInfo.enabledExtensionCount = static_cast<uint32_t>(deviceExtensions.size());
			createInfo.ppEnabledExtensionNames = deviceExtensions.data();

			createInfo.pEnabledFeatures = &deviceFeatures;

			//for older implementations there is a good practice to include validation layers
			if (enableValidationLayers){
				createInfo.enabledLayerCount = static_cast<uint32_t>(validationLayers.size());
			}else{
				createInfo.enabledLayerCount = 0;
			}	

			if(vkCreateDevice(physicalDevice, &createInfo, nullptr, &device)!=VK_SUCCESS){
				throw std::runtime_error("Failed to create logical device!");
			}

			vkGetDeviceQueue(device, indeces.presentFamily.value(), 0, &presentQueue);
		}
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
