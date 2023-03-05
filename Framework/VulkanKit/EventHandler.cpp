#include "EventHandler.hpp"
#include "Window.hpp"

#include <SDL2/SDL_events.h>

bool vulkan::EventHandler::onSwapchainReset(Window&)
{
	return true; /* Propagate to other handlers */
}

bool vulkan::EventHandler::onWindowEvent(Window&, const SDL_Event&)
{
	return true; /* Propagate to other handlers */
}
