#pragma once

#include "InitHelpers.hpp"

#define VULKAN_APPLICATION_INSTANCE(APP) \
namespace vulkan { Application& GetApplication() { \
static APP instance; return instance; } }

namespace vulkan
{

class Window;

class Application : public APIState
{
public:
	void main();
protected:
	virtual void initialize();
	virtual void finalize();
	virtual void runApplication() = 0;
	virtual VkQueue presentQueue() = 0;
	void setOutputWindow(Window* wndPtr);
	virtual DeviceQueueCreateList queueInfos() const = 0;
	virtual bool detectQueues(VkPhysicalDevice physicalDevice) = 0;
	virtual const char* applicationName() const;
	virtual const char* engineName() const;
	explicit Application() = default;
	virtual ~Application() = default;
	bool canPresent(VkPhysicalDevice physicalDevice, uint32_t family) const;
private:
	Window* m_wndPtr = nullptr;
	void initializeInternal();
	void finalizeInternal();
};

}
