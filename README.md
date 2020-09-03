# Vulkan demo apps repositors
Here I am going to place all my demo and test apps written using Vulkan API.

### Application support library: libvkutil
The library is designed to simplify and automate routine tasks, such as instance and device creation. Any application is to be designed as a class inheriting from ***VulkanApp*** base class. The library itself also relies on third-party projects, such as:
* [GLFW](https://github.com/glfw/glfw) library for creation and management of program window, input processing and Vulkan surface creation
* [GLM](https://github.com/g-truc/glm) vector mathematics library for graphics applications
