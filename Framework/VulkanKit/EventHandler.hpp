#pragma once

union SDL_Event;

namespace vulkan
{

class Window;

class EventHandler
{
protected:
	friend class Window;
	virtual bool onSwapchainReset(Window&);
	virtual bool onWindowEvent(Window&, const SDL_Event&);
	EventHandler* m_next = nullptr;
	bool m_enabled = true;
};

}
