CFLAGS = -std=c++17 -O2
LDFLAGS = -lglfw -lvulkan -ldl -lpthread -lX11 -lXxf86vm -lXrandr -lXi

compile: main.cpp
	g++ $(CFLAGS) -o vulkanApp main.cpp $(LDFLAGS)

.PHONY:
	test clean

run: vulkanApp
	./vulkanApp

clean:
	rm -f VulkanApp
