# Vulkan demo apps repository
Here I am going to place all my demo and test apps written using Vulkan API. Everything is written in C++ using custom framework for Vulkan boilerplate code.

### Application support framework: vkutil
The framework is designed to simplify and automate routine tasks, such as instance and device creation. Any application is to be designed as a class inheriting from ***vkutil::Application*** base class. The only other requirement is to implement ***CreateApplication*** function which dynamically allocates and returns a pointer to an instance of ***vkutil::Application***. NOTE: by default, the instance will be deleted using C++ ***delete*** - so, in case memory allocation for the instane has been done differently (e.g. via ***malloc()*** from C library), additional steps are needed: first, define a project-wide ***VKUTIL_APP_CUSTOM_DESTROY*** macro; next, implement ***DestroyApplication()*** function to reclaim memory properly.
