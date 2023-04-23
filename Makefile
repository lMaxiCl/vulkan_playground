CFLAGS = -std=c++17 -O2
LDFLAGS = -lglfw -lvulkan -ldl -lpthread -lX11 -lXxf86vm -lXrandr -lXi

VulkanApp: main.cpp

	g++ $(CFLAGS) -o VulkanTest main.cpp $(LDFLAGS)

.PHONY:
	test clean

test: VulkanTest
	./VulkanTest

clean:
	rm -f VulkanTest