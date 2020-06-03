# Vulkan demo apps repositors
Here I am going to place all my demo and test apps written using Vulkan API.

### Application support library: libvkutil
The library is designed to simplify and automate routine tasks, such as instance and device creation, loading assets etc. The design of the library makes the use of it completely optional (for example, calling ***vkUtilInitialize()*** function will create Vulkan instance, but this step can be skipped - if you have an instance created elsewhere, you let **libvkutil** use it by calling ***vkUtilSetInstance()*** and passing your instance handle as an argument).
