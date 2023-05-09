CFLAGS = -std=c++17 -O2
LDFLAGS = -lglfw -lvulkan -ldl -lpthread -lX11 -lXxf86vm -lXrandr -lXi

compile: main.cpp
	g++ $(CFLAGS) -o vulkanApp main.cpp $(LDFLAGS)

.PHONY:
	test clean

run: vulkanApp
	./vulkanApp __NV_PRIME_RENDER_OFFLOAD=1 vkcube 
clean:
	rm -f VulkanApp
