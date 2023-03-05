#include "ImGuiRenderer.hpp"
#include "PipelineBarrier.hpp"
#include "InitHelpers.hpp"
#include "BindingTable.hpp"

#include "../Utilities/Memory.hpp"
#include "../ImGui/imgui.h"

#include <glm/gtc/matrix_transform.hpp>
#include <SDL2/SDL_events.h>
#include <Sdl2/SDL_clipboard.h>
#include <SDL2/SDL_syswm.h>
#include <SDL2/SDL_timer.h>

namespace vulkan
{

static const uint32_t kImGuiMinBuffers = 2;
static const size_t kImGuiMinVBBytes = 1 MB;

struct ImGuiInternalState
{
	SDL_Window* Window = nullptr;
	Uint64 Time = 0;
	Uint32 MouseWindowID = 0;
	int MouseButtonsDown = 0;
	SDL_Cursor* MouseCursors[ImGuiMouseCursor_COUNT]{};
	SDL_Cursor* LastMouseCursor = nullptr;
	int PendingMouseLeaveFrame = 0;
	char* ClipboardText = nullptr;
};

static const uint32_t ImGuiVertexShader[]
{
	0x07230203,0x00010600,0x0008000a,0x0000002b,0x00000000,0x00020011,0x00000001,0x0006000b,
	0x00000001,0x4c534c47,0x6474732e,0x3035342e,0x00000000,0x0003000e,0x00000000,0x00000001,
	0x000c000f,0x00000000,0x00000004,0x6e69616d,0x00000000,0x0000000d,0x00000013,0x00000019,
	0x00000023,0x00000025,0x00000028,0x00000029,0x00050048,0x0000000b,0x00000000,0x0000000b,
	0x00000000,0x00050048,0x0000000b,0x00000001,0x0000000b,0x00000001,0x00050048,0x0000000b,
	0x00000002,0x0000000b,0x00000003,0x00050048,0x0000000b,0x00000003,0x0000000b,0x00000004,
	0x00030047,0x0000000b,0x00000002,0x00040048,0x00000011,0x00000000,0x00000005,0x00050048,
	0x00000011,0x00000000,0x00000023,0x00000000,0x00050048,0x00000011,0x00000000,0x00000007,
	0x00000010,0x00030047,0x00000011,0x00000002,0x00040047,0x00000019,0x0000001e,0x00000000,
	0x00040047,0x00000023,0x0000001e,0x00000000,0x00040047,0x00000025,0x0000001e,0x00000002,
	0x00040047,0x00000028,0x0000001e,0x00000001,0x00040047,0x00000029,0x0000001e,0x00000001,
	0x00020013,0x00000002,0x00030021,0x00000003,0x00000002,0x00030016,0x00000006,0x00000020,
	0x00040017,0x00000007,0x00000006,0x00000004,0x00040015,0x00000008,0x00000020,0x00000000,
	0x0004002b,0x00000008,0x00000009,0x00000001,0x0004001c,0x0000000a,0x00000006,0x00000009,
	0x0006001e,0x0000000b,0x00000007,0x00000006,0x0000000a,0x0000000a,0x00040020,0x0000000c,
	0x00000003,0x0000000b,0x0004003b,0x0000000c,0x0000000d,0x00000003,0x00040015,0x0000000e,
	0x00000020,0x00000001,0x0004002b,0x0000000e,0x0000000f,0x00000000,0x00040018,0x00000010,
	0x00000007,0x00000004,0x0003001e,0x00000011,0x00000010,0x00040020,0x00000012,0x00000009,
	0x00000011,0x0004003b,0x00000012,0x00000013,0x00000009,0x00040020,0x00000014,0x00000009,
	0x00000010,0x00040017,0x00000017,0x00000006,0x00000002,0x00040020,0x00000018,0x00000001,
	0x00000017,0x0004003b,0x00000018,0x00000019,0x00000001,0x0004002b,0x00000006,0x0000001b,
	0x00000000,0x0004002b,0x00000006,0x0000001c,0x3f800000,0x00040020,0x00000021,0x00000003,
	0x00000007,0x0004003b,0x00000021,0x00000023,0x00000003,0x00040020,0x00000024,0x00000001,
	0x00000007,0x0004003b,0x00000024,0x00000025,0x00000001,0x00040020,0x00000027,0x00000003,
	0x00000017,0x0004003b,0x00000027,0x00000028,0x00000003,0x0004003b,0x00000018,0x00000029,
	0x00000001,0x00050036,0x00000002,0x00000004,0x00000000,0x00000003,0x000200f8,0x00000005,
	0x00050041,0x00000014,0x00000015,0x00000013,0x0000000f,0x0004003d,0x00000010,0x00000016,
	0x00000015,0x0004003d,0x00000017,0x0000001a,0x00000019,0x00050051,0x00000006,0x0000001d,
	0x0000001a,0x00000000,0x00050051,0x00000006,0x0000001e,0x0000001a,0x00000001,0x00070050,
	0x00000007,0x0000001f,0x0000001d,0x0000001e,0x0000001b,0x0000001c,0x00050091,0x00000007,
	0x00000020,0x00000016,0x0000001f,0x00050041,0x00000021,0x00000022,0x0000000d,0x0000000f,
	0x0003003e,0x00000022,0x00000020,0x0004003d,0x00000007,0x00000026,0x00000025,0x0003003e,
	0x00000023,0x00000026,0x0004003d,0x00000017,0x0000002a,0x00000029,0x0003003e,0x00000028,
	0x0000002a,0x000100fd,0x00010038
};

static const uint32_t ImGuiFragmentShader[]
{
	0x07230203,0x00010600,0x0008000a,0x00000025,0x00000000,0x00020011,0x00000001,0x0006000b,
	0x00000001,0x4c534c47,0x6474732e,0x3035342e,0x00000000,0x0003000e,0x00000000,0x00000001,
	0x0009000f,0x00000004,0x00000004,0x6e69616d,0x00000000,0x00000009,0x0000000b,0x00000017,
	0x0000001b,0x00030010,0x00000004,0x00000007,0x00040047,0x00000009,0x0000001e,0x00000000,
	0x00040047,0x0000000b,0x0000001e,0x00000000,0x00040047,0x00000017,0x00000022,0x00000000,
	0x00040047,0x00000017,0x00000021,0x00000000,0x00040047,0x0000001b,0x0000001e,0x00000001,
	0x00020013,0x00000002,0x00030021,0x00000003,0x00000002,0x00030016,0x00000006,0x00000020,
	0x00040017,0x00000007,0x00000006,0x00000004,0x00040020,0x00000008,0x00000003,0x00000007,
	0x0004003b,0x00000008,0x00000009,0x00000003,0x00040020,0x0000000a,0x00000001,0x00000007,
	0x0004003b,0x0000000a,0x0000000b,0x00000001,0x00040017,0x0000000c,0x00000006,0x00000003,
	0x00040015,0x0000000f,0x00000020,0x00000000,0x0004002b,0x0000000f,0x00000010,0x00000003,
	0x00040020,0x00000011,0x00000001,0x00000006,0x00090019,0x00000014,0x00000006,0x00000001,
	0x00000000,0x00000000,0x00000000,0x00000001,0x00000000,0x0003001b,0x00000015,0x00000014,
	0x00040020,0x00000016,0x00000000,0x00000015,0x0004003b,0x00000016,0x00000017,0x00000000,
	0x00040017,0x00000019,0x00000006,0x00000002,0x00040020,0x0000001a,0x00000001,0x00000019,
	0x0004003b,0x0000001a,0x0000001b,0x00000001,0x0004002b,0x0000000f,0x0000001e,0x00000000,
	0x00050036,0x00000002,0x00000004,0x00000000,0x00000003,0x000200f8,0x00000005,0x0004003d,
	0x00000007,0x0000000d,0x0000000b,0x0008004f,0x0000000c,0x0000000e,0x0000000d,0x0000000d,
	0x00000000,0x00000001,0x00000002,0x00050041,0x00000011,0x00000012,0x0000000b,0x00000010,
	0x0004003d,0x00000006,0x00000013,0x00000012,0x0004003d,0x00000015,0x00000018,0x00000017,
	0x0004003d,0x00000019,0x0000001c,0x0000001b,0x00050057,0x00000007,0x0000001d,0x00000018,
	0x0000001c,0x00050051,0x00000006,0x0000001f,0x0000001d,0x00000000,0x00050085,0x00000006,
	0x00000020,0x00000013,0x0000001f,0x00050051,0x00000006,0x00000021,0x0000000e,0x00000000,
	0x00050051,0x00000006,0x00000022,0x0000000e,0x00000001,0x00050051,0x00000006,0x00000023,
	0x0000000e,0x00000002,0x00070050,0x00000007,0x00000024,0x00000021,0x00000022,0x00000023,
	0x00000020,0x0003003e,0x00000009,0x00000024,0x000100fd,0x00010038
};

static void ImGuiSetClipboardText(void* context, const char* text)
{
	SDL_SetClipboardText(text);
}

static void ImGuiSetPlatformImeData(ImGuiViewport*, ImGuiPlatformImeData* data)
{
	if (!data->WantVisible)
	{
		SDL_StopTextInput();
		return;
	}
	SDL_Rect r{ int(data->InputPos.x), int(data->InputPos.y), 1, int(data->InputLineHeight) };
	SDL_SetTextInputRect(&r);
	SDL_StartTextInput();
}

static const char* ImGuiGetClipboardText(void* context)
{
	char** dataRef = reinterpret_cast<char**>(context);
	if (*dataRef) SDL_free(*dataRef);
	*dataRef = SDL_GetClipboardText();
	return *dataRef;
}

static ImGuiKey ImGuiKeycodeToImGuiKey(int keycode)
{
	switch (keycode)
	{
	case SDLK_TAB: return ImGuiKey_Tab;
	case SDLK_LEFT: return ImGuiKey_LeftArrow;
	case SDLK_RIGHT: return ImGuiKey_RightArrow;
	case SDLK_UP: return ImGuiKey_UpArrow;
	case SDLK_DOWN: return ImGuiKey_DownArrow;
	case SDLK_PAGEUP: return ImGuiKey_PageUp;
	case SDLK_PAGEDOWN: return ImGuiKey_PageDown;
	case SDLK_HOME: return ImGuiKey_Home;
	case SDLK_END: return ImGuiKey_End;
	case SDLK_INSERT: return ImGuiKey_Insert;
	case SDLK_DELETE: return ImGuiKey_Delete;
	case SDLK_BACKSPACE: return ImGuiKey_Backspace;
	case SDLK_SPACE: return ImGuiKey_Space;
	case SDLK_RETURN: return ImGuiKey_Enter;
	case SDLK_ESCAPE: return ImGuiKey_Escape;
	case SDLK_QUOTE: return ImGuiKey_Apostrophe;
	case SDLK_COMMA: return ImGuiKey_Comma;
	case SDLK_MINUS: return ImGuiKey_Minus;
	case SDLK_PERIOD: return ImGuiKey_Period;
	case SDLK_SLASH: return ImGuiKey_Slash;
	case SDLK_SEMICOLON: return ImGuiKey_Semicolon;
	case SDLK_EQUALS: return ImGuiKey_Equal;
	case SDLK_LEFTBRACKET: return ImGuiKey_LeftBracket;
	case SDLK_BACKSLASH: return ImGuiKey_Backslash;
	case SDLK_RIGHTBRACKET: return ImGuiKey_RightBracket;
	case SDLK_BACKQUOTE: return ImGuiKey_GraveAccent;
	case SDLK_CAPSLOCK: return ImGuiKey_CapsLock;
	case SDLK_SCROLLLOCK: return ImGuiKey_ScrollLock;
	case SDLK_NUMLOCKCLEAR: return ImGuiKey_NumLock;
	case SDLK_PRINTSCREEN: return ImGuiKey_PrintScreen;
	case SDLK_PAUSE: return ImGuiKey_Pause;
	case SDLK_KP_0: return ImGuiKey_Keypad0;
	case SDLK_KP_1: return ImGuiKey_Keypad1;
	case SDLK_KP_2: return ImGuiKey_Keypad2;
	case SDLK_KP_3: return ImGuiKey_Keypad3;
	case SDLK_KP_4: return ImGuiKey_Keypad4;
	case SDLK_KP_5: return ImGuiKey_Keypad5;
	case SDLK_KP_6: return ImGuiKey_Keypad6;
	case SDLK_KP_7: return ImGuiKey_Keypad7;
	case SDLK_KP_8: return ImGuiKey_Keypad8;
	case SDLK_KP_9: return ImGuiKey_Keypad9;
	case SDLK_KP_PERIOD: return ImGuiKey_KeypadDecimal;
	case SDLK_KP_DIVIDE: return ImGuiKey_KeypadDivide;
	case SDLK_KP_MULTIPLY: return ImGuiKey_KeypadMultiply;
	case SDLK_KP_MINUS: return ImGuiKey_KeypadSubtract;
	case SDLK_KP_PLUS: return ImGuiKey_KeypadAdd;
	case SDLK_KP_ENTER: return ImGuiKey_KeypadEnter;
	case SDLK_KP_EQUALS: return ImGuiKey_KeypadEqual;
	case SDLK_LCTRL: return ImGuiKey_LeftCtrl;
	case SDLK_LSHIFT: return ImGuiKey_LeftShift;
	case SDLK_LALT: return ImGuiKey_LeftAlt;
	case SDLK_LGUI: return ImGuiKey_LeftSuper;
	case SDLK_RCTRL: return ImGuiKey_RightCtrl;
	case SDLK_RSHIFT: return ImGuiKey_RightShift;
	case SDLK_RALT: return ImGuiKey_RightAlt;
	case SDLK_RGUI: return ImGuiKey_RightSuper;
	case SDLK_APPLICATION: return ImGuiKey_Menu;
	case SDLK_0: return ImGuiKey_0;
	case SDLK_1: return ImGuiKey_1;
	case SDLK_2: return ImGuiKey_2;
	case SDLK_3: return ImGuiKey_3;
	case SDLK_4: return ImGuiKey_4;
	case SDLK_5: return ImGuiKey_5;
	case SDLK_6: return ImGuiKey_6;
	case SDLK_7: return ImGuiKey_7;
	case SDLK_8: return ImGuiKey_8;
	case SDLK_9: return ImGuiKey_9;
	case SDLK_a: return ImGuiKey_A;
	case SDLK_b: return ImGuiKey_B;
	case SDLK_c: return ImGuiKey_C;
	case SDLK_d: return ImGuiKey_D;
	case SDLK_e: return ImGuiKey_E;
	case SDLK_f: return ImGuiKey_F;
	case SDLK_g: return ImGuiKey_G;
	case SDLK_h: return ImGuiKey_H;
	case SDLK_i: return ImGuiKey_I;
	case SDLK_j: return ImGuiKey_J;
	case SDLK_k: return ImGuiKey_K;
	case SDLK_l: return ImGuiKey_L;
	case SDLK_m: return ImGuiKey_M;
	case SDLK_n: return ImGuiKey_N;
	case SDLK_o: return ImGuiKey_O;
	case SDLK_p: return ImGuiKey_P;
	case SDLK_q: return ImGuiKey_Q;
	case SDLK_r: return ImGuiKey_R;
	case SDLK_s: return ImGuiKey_S;
	case SDLK_t: return ImGuiKey_T;
	case SDLK_u: return ImGuiKey_U;
	case SDLK_v: return ImGuiKey_V;
	case SDLK_w: return ImGuiKey_W;
	case SDLK_x: return ImGuiKey_X;
	case SDLK_y: return ImGuiKey_Y;
	case SDLK_z: return ImGuiKey_Z;
	case SDLK_F1: return ImGuiKey_F1;
	case SDLK_F2: return ImGuiKey_F2;
	case SDLK_F3: return ImGuiKey_F3;
	case SDLK_F4: return ImGuiKey_F4;
	case SDLK_F5: return ImGuiKey_F5;
	case SDLK_F6: return ImGuiKey_F6;
	case SDLK_F7: return ImGuiKey_F7;
	case SDLK_F8: return ImGuiKey_F8;
	case SDLK_F9: return ImGuiKey_F9;
	case SDLK_F10: return ImGuiKey_F10;
	case SDLK_F11: return ImGuiKey_F11;
	case SDLK_F12: return ImGuiKey_F12;
	}
	return ImGuiKey_None;
}

static void ImGuiUpdateKeyModifiers(SDL_Keymod sdl_key_mods)
{
	ImGuiIO& io = ImGui::GetIO();
	io.AddKeyEvent(ImGuiMod_Ctrl, (sdl_key_mods & KMOD_CTRL) != 0);
	io.AddKeyEvent(ImGuiMod_Shift, (sdl_key_mods & KMOD_SHIFT) != 0);
	io.AddKeyEvent(ImGuiMod_Alt, (sdl_key_mods & KMOD_ALT) != 0);
	io.AddKeyEvent(ImGuiMod_Super, (sdl_key_mods & KMOD_GUI) != 0);
}

static void ImGuiUpdateMouseData(ImGuiInternalState* state)
{
	ImGuiIO& io = ImGui::GetIO();
#if SDL_HAS_CAPTURE_AND_GLOBAL_MOUSE
	SDL_CaptureMouse((states->MouseButtonsDown != 0) ? SDL_TRUE : SDL_FALSE);
	SDL_Window* focused_window = SDL_GetKeyboardFocus();
	const bool is_app_focused = (bd->Window == focused_window);
#else
	const bool is_app_focused = (SDL_GetWindowFlags(state->Window) & SDL_WINDOW_INPUT_FOCUS) != 0;
#endif
	if (is_app_focused)
	{
		if (io.WantSetMousePos) SDL_WarpMouseInWindow(state->Window, (int)io.MousePos.x, (int)io.MousePos.y);
		if (state->MouseButtonsDown == 0)
		{
			int window_x, window_y, mouse_x_global, mouse_y_global;
			SDL_GetGlobalMouseState(&mouse_x_global, &mouse_y_global);
			SDL_GetWindowPosition(state->Window, &window_x, &window_y);
			io.AddMousePosEvent((float)(mouse_x_global - window_x), (float)(mouse_y_global - window_y));
		}
	}
}

static void ImGuiUpdateMouseCursor(ImGuiInternalState* state)
{
	ImGuiIO& io = ImGui::GetIO();
	ImGuiMouseCursor cursor = ImGui::GetMouseCursor();
	if (io.ConfigFlags & ImGuiConfigFlags_NoMouseCursorChange) return;
	if (io.MouseDrawCursor || cursor == ImGuiMouseCursor_None) SDL_ShowCursor(SDL_FALSE);	
	else
	{
		SDL_Cursor* expected = state->MouseCursors[cursor] ? state->MouseCursors[cursor] : state->MouseCursors[ImGuiMouseCursor_Arrow];
		if (state->LastMouseCursor != expected)
		{
			SDL_SetCursor(expected);
			state->LastMouseCursor = expected;
		}
		SDL_ShowCursor(SDL_TRUE);
	}
}

}

vulkan::ImGuiRenderer::ImGuiRenderer(const APIState& vk)
	: APIClient(vk), m_bindingTableLayout{ vk }
{
}

void vulkan::ImGuiRenderer::initialize(const Config& conf)
{
	ImGui::CreateContext();
	ImGuiIO& io = ImGui::GetIO();
	m_int = IM_NEW(ImGuiInternalState);
	io.BackendPlatformUserData = m_int;
	io.BackendPlatformName = "vulkan::ImGuiRenderer";
	io.BackendFlags |= ImGuiBackendFlags_HasMouseCursors;
	io.BackendFlags |= ImGuiBackendFlags_HasSetMousePos;

	io.ClipboardUserData = &m_int->ClipboardText;
	io.SetClipboardTextFn = &ImGuiSetClipboardText;
	io.GetClipboardTextFn = &ImGuiGetClipboardText;
	io.SetPlatformImeDataFn = &ImGuiSetPlatformImeData;

	m_int->MouseCursors[ImGuiMouseCursor_Arrow] = SDL_CreateSystemCursor(SDL_SYSTEM_CURSOR_ARROW);
	m_int->MouseCursors[ImGuiMouseCursor_TextInput] = SDL_CreateSystemCursor(SDL_SYSTEM_CURSOR_IBEAM);
	m_int->MouseCursors[ImGuiMouseCursor_ResizeAll] = SDL_CreateSystemCursor(SDL_SYSTEM_CURSOR_SIZEALL);
	m_int->MouseCursors[ImGuiMouseCursor_ResizeNS] = SDL_CreateSystemCursor(SDL_SYSTEM_CURSOR_SIZENS);
	m_int->MouseCursors[ImGuiMouseCursor_ResizeEW] = SDL_CreateSystemCursor(SDL_SYSTEM_CURSOR_SIZEWE);
	m_int->MouseCursors[ImGuiMouseCursor_ResizeNESW] = SDL_CreateSystemCursor(SDL_SYSTEM_CURSOR_SIZENESW);
	m_int->MouseCursors[ImGuiMouseCursor_ResizeNWSE] = SDL_CreateSystemCursor(SDL_SYSTEM_CURSOR_SIZENWSE);
	m_int->MouseCursors[ImGuiMouseCursor_Hand] = SDL_CreateSystemCursor(SDL_SYSTEM_CURSOR_HAND);
	m_int->MouseCursors[ImGuiMouseCursor_NotAllowed] = SDL_CreateSystemCursor(SDL_SYSTEM_CURSOR_NO);

	m_int->Window = conf.window;
	ImGuiViewport* mainViewport = ImGui::GetMainViewport();
	mainViewport->PlatformHandleRaw = nullptr;
	SDL_SysWMinfo info;
	SDL_VERSION(&info.version);
	if (SDL_GetWindowWMInfo(conf.window, &info))
	{
#if defined(SDL_VIDEO_DRIVER_WINDOWS)
		mainViewport->PlatformHandleRaw = (void*)info.info.win.window;
#elif defined(__APPLE__) && defined(SDL_VIDEO_DRIVER_COCOA)
		mainViewport->PlatformHandleRaw = (void*)info.info.cocoa.window;
#endif
	}
#ifdef SDL_HINT_MOUSE_FOCUS_CLICKTHROUGH
	SDL_SetHint(SDL_HINT_MOUSE_FOCUS_CLICKTHROUGH, "1");
#endif
#ifdef SDL_HINT_IME_SHOW_UI
	SDL_SetHint(SDL_HINT_IME_SHOW_UI, "1");
#endif
#ifdef SDL_HINT_MOUSE_AUTO_CAPTURE
	SDL_SetHint(SDL_HINT_MOUSE_AUTO_CAPTURE, "0");
#endif

	m_bufferBytes = Max(conf.vertexMem, kImGuiMinVBBytes);
	uint32_t numBuffers = Max(conf.numBuffers, kImGuiMinBuffers);
	m_vertexBuffers = Array<VkBuffer>::New(numBuffers);
	m_currentBuffer = numBuffers - 1;

	vulkan::PhysicalDeviceMemoryProperties memProps(conf.physicalDevice);
	vulkan::BufferCreateInfo bci{ m_bufferBytes, VK_BUFFER_USAGE_VERTEX_BUFFER_BIT | VK_BUFFER_USAGE_INDEX_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_SRC_BIT };
	for (VkBuffer& buffer : m_vertexBuffers) BreakIfFailed(vkCreateBuffer(m_vk.device(), &bci, m_vk.alloc(), &buffer));

	VkMemoryRequirements bufferMemReq;
	vkGetBufferMemoryRequirements(m_vk.device(), m_vertexBuffers[0], &bufferMemReq);
	m_bufferBytes = AlignUp(bufferMemReq.size, bufferMemReq.alignment);

	vulkan::MemoryAllocateInfo bmai{ m_bufferBytes * m_vertexBuffers.num, memProps.findMemoryType(bufferMemReq, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT, 0, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT) };
	BreakIfFailed(vkAllocateMemory(m_vk.device(), &bmai, m_vk.alloc(), &m_bufferMem));
	for (uint32_t i = 0; i < m_vertexBuffers.num; i++) BreakIfFailed(vkBindBufferMemory(m_vk.device(), m_vertexBuffers[i], m_bufferMem, i * m_bufferBytes));

	int fontW = 0, fontH = 0;
	unsigned char* fontData = nullptr;
	ImGui::GetIO().Fonts->GetTexDataAsAlpha8(&fontData, &fontW, &fontH);
	BreakIfNot((fontW * fontH) <= m_bufferBytes);

	void* bufferBase = nullptr;
	vulkan::MappedMemoryRange range{ m_bufferMem, m_currentBuffer * m_bufferBytes, m_bufferBytes };
	BreakIfFailed(vkMapMemory(m_vk.device(), m_bufferMem, range.offset, range.size, 0, &bufferBase));
	memcpy(bufferBase, fontData, (fontW * fontH));
	BreakIfFailed(vkFlushMappedMemoryRanges(m_vk.device(), 1, &range));
	vkUnmapMemory(m_vk.device(), m_bufferMem);

	const VkExtent2D extent{ uint32_t(fontW), uint32_t(fontH) };
	vulkan::Image2DCreateInfo ici{ VK_FORMAT_R8_UNORM, extent, 1, 1, VK_SAMPLE_COUNT_1_BIT, VK_IMAGE_TILING_OPTIMAL, VK_IMAGE_USAGE_SAMPLED_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT };
	BreakIfFailed(vkCreateImage(m_vk.device(), &ici, m_vk.alloc(), &m_texture));

	VkMemoryRequirements textureMemReq;
	vkGetImageMemoryRequirements(m_vk.device(), m_texture, &textureMemReq);
	BreakIfFailed(textureMemReq.size > m_bufferBytes);

	vulkan::MemoryAllocateInfo imai{ textureMemReq.size, memProps.findMemoryType(textureMemReq, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT) };
	BreakIfFailed(vkAllocateMemory(m_vk.device(), &imai, m_vk.alloc(), &m_imageMem));
	BreakIfFailed(vkBindImageMemory(m_vk.device(), m_texture, m_imageMem, 0));

	vulkan::ShaderModuleCreateInfo smciVert{ sizeof(ImGuiVertexShader), ImGuiVertexShader };
	vulkan::ShaderModuleCreateInfo smciFrag{ sizeof(ImGuiFragmentShader), ImGuiFragmentShader };
	BreakIfFailed(vkCreateShaderModule(m_vk.device(), &smciVert, m_vk.alloc(), &m_shaderVert));
	BreakIfFailed(vkCreateShaderModule(m_vk.device(), &smciFrag, m_vk.alloc(), &m_shaderFrag));

	VkSamplerCreateInfo sci{ VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO };
	BreakIfFailed(vkCreateSampler(m_vk.device(), &sci, m_vk.alloc(), &m_sampler));

	m_bindingTableLayout.initialize(
		{
			{ 
				{ 0, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 1, VK_SHADER_STAGE_FRAGMENT_BIT } 
			}
		},
		{
			{ VK_SHADER_STAGE_VERTEX_BIT, 0, 64 }
		});
	m_bindingTableLayout.createBindingTable(m_bindingTable);
}

void vulkan::ImGuiRenderer::startNewFrame(VkCommandBuffer commandBuffer, const VkRect2D& rect)
{
	ImGuiIO& io = ImGui::GetIO();
	io.DisplaySize = ImVec2{ float(rect.extent.width), float(rect.extent.height) };

	// Setup display size (every frame to accommodate for window resizing)
	// io.DisplayFramebufferScale = ImVec2((float)display_w / w, (float)display_h / h);

	Uint64 current_time = SDL_GetPerformanceCounter();
	static Uint64 frequency = SDL_GetPerformanceFrequency();
	io.DeltaTime = m_int->Time > 0 ? float(double(current_time - m_int->Time) / frequency) : float(1.0f / 60.0f);
	m_int->Time = current_time;

	if (m_int->PendingMouseLeaveFrame && m_int->PendingMouseLeaveFrame >= ImGui::GetFrameCount() && m_int->MouseButtonsDown == 0)
	{
		m_int->MouseWindowID = 0;
		m_int->PendingMouseLeaveFrame = 0;
		io.AddMousePosEvent(-FLT_MAX, -FLT_MAX);
	}

	ImGuiUpdateMouseData(m_int);
	ImGuiUpdateMouseCursor(m_int);

	if (m_srv == VK_NULL_HANDLE)
	{
		int fontW = 0, fontH = 0;
		unsigned char* fontData = nullptr;
		io.Fonts->GetTexDataAsAlpha8(&fontData, &fontW, &fontH);

		vulkan::PipelineBarrier barrier{};
		barrier.image(m_texture, { VK_IMAGE_ASPECT_COLOR_BIT, 0, 1, 0, 1 })
			.layout(VK_IMAGE_LAYOUT_UNDEFINED, 0, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, VK_ACCESS_TRANSFER_WRITE_BIT);
		barrier.submit(commandBuffer, VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT, VK_PIPELINE_STAGE_TRANSFER_BIT);

		VkBuffer src = m_vertexBuffers[m_currentBuffer];
		VkBufferImageCopy bic{ 0, 0, 0, { VK_IMAGE_ASPECT_COLOR_BIT, 0, 0, 1 }, {}, { uint32_t(fontW), uint32_t(fontH), 1 } };
		vkCmdCopyBufferToImage(commandBuffer, src, m_texture, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 1, &bic);

		barrier.image(0).layout(VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, VK_ACCESS_TRANSFER_WRITE_BIT, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL, VK_ACCESS_SHADER_READ_BIT);
		barrier.submit(commandBuffer, VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT);

		vulkan::Image2DViewCreateInfo ivci{ m_texture, VK_FORMAT_R8_UNORM, {}, { VK_IMAGE_ASPECT_COLOR_BIT, 0, 1, 0, 1 } };
		BreakIfFailed(vkCreateImageView(m_vk.device(), &ivci, m_vk.alloc(), &m_srv));

		VkDescriptorImageInfo dii{ m_sampler, m_srv, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL };
		m_bindingTable.bindTexture(0, 0, dii).applyPendingUpdates(m_vk.device());
	}

	ImGui::NewFrame();
}

void vulkan::ImGuiRenderer::render(VkCommandBuffer commandBuffer, VkImageView renderTarget, VkFormat outputFormat, const VkClearColorValue* clearColor)
{
	ImGui::Render();
	ImGuiIO& io = ImGui::GetIO();
	ImDrawData* drawData = ImGui::GetDrawData();
	const bool hasData = (drawData->TotalVtxCount > 0);
	const bool startPass = (renderTarget != VK_NULL_HANDLE);
	if (startPass)
	{
		const VkRenderingAttachmentInfo raiColor
		{
			VK_STRUCTURE_TYPE_RENDERING_ATTACHMENT_INFO, nullptr, renderTarget, VK_IMAGE_LAYOUT_ATTACHMENT_OPTIMAL,
			VK_RESOLVE_MODE_NONE, VK_NULL_HANDLE, VK_IMAGE_LAYOUT_UNDEFINED, clearColor ? VK_ATTACHMENT_LOAD_OP_CLEAR : VK_ATTACHMENT_LOAD_OP_LOAD,
			VK_ATTACHMENT_STORE_OP_STORE, { clearColor ? *clearColor : VkClearColorValue{} }
		};
		const VkRect2D rect{ {}, { uint32_t(io.DisplaySize.x), uint32_t(io.DisplaySize.y) } };
		const VkRenderingInfo ri{ VK_STRUCTURE_TYPE_RENDERING_INFO, nullptr, 0, rect, 1, 0, 1, &raiColor };
		vkCmdBeginRendering(commandBuffer, &ri);
	}
	if (hasData)
	{
		void* bufferBase = nullptr;
		m_currentBuffer = (m_currentBuffer + 1) % m_vertexBuffers.num;
		vulkan::MappedMemoryRange range{ m_bufferMem, m_currentBuffer * m_bufferBytes, m_bufferBytes };
		BreakIfFailed(vkMapMemory(m_vk.device(), m_bufferMem, range.offset, range.size, 0, &bufferBase));

		ImDrawVert* vertex = static_cast<ImDrawVert*>(bufferBase);
		VkDeviceSize indexOffset = AlignUp<VkDeviceSize>(drawData->TotalVtxCount * sizeof(ImDrawVert), 16), vertexOffset = 0;
		ImDrawIdx* index = reinterpret_cast<ImDrawIdx*>(static_cast<char*>(bufferBase) + indexOffset);
		for (int i = 0; i < drawData->CmdListsCount; i++)
		{
			const ImDrawList& cmdList = *drawData->CmdLists[i];
			memcpy(vertex, cmdList.VtxBuffer.Data, cmdList.VtxBuffer.Size * sizeof(ImDrawVert));
			memcpy(index, cmdList.IdxBuffer.Data, cmdList.IdxBuffer.Size * sizeof(ImDrawIdx));
			vertex += cmdList.VtxBuffer.Size;
			index += cmdList.IdxBuffer.Size;
		}

		BreakIfFailed(vkFlushMappedMemoryRanges(m_vk.device(), 1, &range));
		vkUnmapMemory(m_vk.device(), m_bufferMem);

		const float l = drawData->DisplayPos.x;
		const float t = drawData->DisplayPos.y;
		const float r = drawData->DisplayPos.x + drawData->DisplaySize.x;
		const float b = drawData->DisplayPos.y + drawData->DisplaySize.y;
		glm::mat4 transform = glm::ortho(l, r, t, b);

		m_bindingTable.bindForGraphics(commandBuffer);

		vkCmdBindVertexBuffers(commandBuffer, 0, 1, &m_vertexBuffers[m_currentBuffer], &vertexOffset);
		vkCmdBindIndexBuffer(commandBuffer, m_vertexBuffers[m_currentBuffer], indexOffset, sizeof(ImDrawIdx) == 2 ? VK_INDEX_TYPE_UINT16 : VK_INDEX_TYPE_UINT32);
		vkCmdPushConstants(commandBuffer, m_bindingTable.layout(), VK_SHADER_STAGE_VERTEX_BIT, 0, sizeof(transform), &transform);
		vkCmdBindPipeline(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline(outputFormat));

		const VkViewport viewport{ 0.f, 0.f, io.DisplaySize.x, io.DisplaySize.y, 0.f, 1.f };
		vkCmdSetViewport(commandBuffer, 0, 1, &viewport);

		uint32_t vbOffs = 0, ibOffs = 0;
		ImVec2& noClip = drawData->DisplayPos;
		for (int i = 0; i < drawData->CmdListsCount; i++)
		{
			const ImDrawList& cmdList = *drawData->CmdLists[i];
			for (int j = 0; j < cmdList.CmdBuffer.Size; j++)
			{
				const ImDrawCmd& cmd = cmdList.CmdBuffer[j];
				const VkRect2D scissor
				{
					{ int(cmd.ClipRect.x - noClip.x), int(cmd.ClipRect.y - noClip.y) },
					{ uint32_t(cmd.ClipRect.z - cmd.ClipRect.x), uint32_t(cmd.ClipRect.w - cmd.ClipRect.y)}
				};
				vkCmdSetScissor(commandBuffer, 0, 1, &scissor);
				vkCmdDrawIndexed(commandBuffer, cmd.ElemCount, 1, cmd.IdxOffset + ibOffs, cmd.VtxOffset + vbOffs, 0);
			}
			ibOffs += cmdList.IdxBuffer.Size;
			vbOffs += cmdList.VtxBuffer.Size;
		}
	}
	if (startPass) vkCmdEndRendering(commandBuffer);
}

void vulkan::ImGuiRenderer::finalize()
{
	ImGui::DestroyContext();
	m_bindingTableLayout.finalize();
	if (m_int->ClipboardText) SDL_free(m_int->ClipboardText);
	for (VkBuffer& buffer : m_vertexBuffers) vkDestroyBuffer(m_vk.device(), buffer, m_vk.alloc());
	vkDestroyShaderModule(m_vk.device(), m_shaderVert, m_vk.alloc());
	vkDestroyShaderModule(m_vk.device(), m_shaderFrag, m_vk.alloc());
	vkDestroyPipeline(m_vk.device(), m_pipeline, m_vk.alloc());
	vkDestroySampler(m_vk.device(), m_sampler, m_vk.alloc());
	vkDestroyImageView(m_vk.device(), m_srv, m_vk.alloc());
	vkDestroyImage(m_vk.device(), m_texture, m_vk.alloc());
	vkFreeMemory(m_vk.device(), m_bufferMem, m_vk.alloc());
	vkFreeMemory(m_vk.device(), m_imageMem, m_vk.alloc());
	IM_DELETE(m_int);
}

bool vulkan::ImGuiRenderer::onWindowEvent(Window& window, const SDL_Event& event)
{
	ImGuiIO& io = ImGui::GetIO();
	switch (event.type)
	{
	case SDL_MOUSEMOTION:
	{
		ImVec2 mouse_pos((float)event.motion.x, (float)event.motion.y);
		io.AddMousePosEvent(mouse_pos.x, mouse_pos.y);
	}
	break;
	case SDL_MOUSEWHEEL:
	{
		float wheel_x = -event.wheel.preciseX;
		float wheel_y = event.wheel.preciseY;
		io.AddMouseWheelEvent(wheel_x, wheel_y);
	}
	break;
	case SDL_MOUSEBUTTONDOWN:
		[[fallthrough]];
	case SDL_MOUSEBUTTONUP:
	{
		int mouse_button = -1;
		if (event.button.button == SDL_BUTTON_LEFT) mouse_button = 0;
		if (event.button.button == SDL_BUTTON_RIGHT) mouse_button = 1;
		if (event.button.button == SDL_BUTTON_MIDDLE) mouse_button = 2;
		if (event.button.button == SDL_BUTTON_X1) mouse_button = 3;
		if (event.button.button == SDL_BUTTON_X2) mouse_button = 4;
		if (mouse_button == -1) break;
		io.AddMouseButtonEvent(mouse_button, (event.type == SDL_MOUSEBUTTONDOWN));
		m_int->MouseButtonsDown = (event.type == SDL_MOUSEBUTTONDOWN) ? (m_int->MouseButtonsDown | (1 << mouse_button)) : (m_int->MouseButtonsDown & ~(1 << mouse_button));
	}
	break;
	case SDL_TEXTINPUT:
	{
		io.AddInputCharactersUTF8(event.text.text);
	}
	break;
	case SDL_KEYDOWN:
		[[fallthrough]];
	case SDL_KEYUP:
	{
		ImGuiUpdateKeyModifiers((SDL_Keymod)event.key.keysym.mod);
		ImGuiKey key = ImGuiKeycodeToImGuiKey(event.key.keysym.sym);
		io.AddKeyEvent(key, (event.type == SDL_KEYDOWN));
		io.SetKeyEventNativeData(key, event.key.keysym.sym, event.key.keysym.scancode, event.key.keysym.scancode);
	}
	break;
	case SDL_WINDOWEVENT:
	{
		Uint8 window_event = event.window.event;
		if (window_event == SDL_WINDOWEVENT_ENTER)
		{
			m_int->MouseWindowID = event.window.windowID;
			m_int->PendingMouseLeaveFrame = 0;
		}
		if (window_event == SDL_WINDOWEVENT_LEAVE) m_int->PendingMouseLeaveFrame = ImGui::GetFrameCount() + 1;
		if (window_event == SDL_WINDOWEVENT_FOCUS_GAINED) io.AddFocusEvent(true);
		else if (event.window.event == SDL_WINDOWEVENT_FOCUS_LOST) io.AddFocusEvent(false);
	}
	break;
	}
	return !io.WantCaptureMouse;
}

VkPipeline vulkan::ImGuiRenderer::pipeline(VkFormat outputFormat)
{
	if (m_pipeline == VK_NULL_HANDLE || m_outputFormat != outputFormat)
	{
		m_outputFormat = outputFormat;
		if (m_pipeline != VK_NULL_HANDLE) vkDestroyPipeline(m_vk.device(), m_pipeline, m_vk.alloc());
		VkPipelineShaderStageCreateInfo pssci[]
		{
			{ VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO, nullptr, 0, VK_SHADER_STAGE_VERTEX_BIT, m_shaderVert, "main" },
			{ VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO, nullptr, 0, VK_SHADER_STAGE_FRAGMENT_BIT, m_shaderFrag, "main" },
		};
		VkVertexInputBindingDescription vibd{ 0, sizeof(ImDrawVert), VK_VERTEX_INPUT_RATE_VERTEX };
		VkVertexInputAttributeDescription viad[]
		{
			{ 0, 0, VK_FORMAT_R32G32_SFLOAT, 0 }, { 1, 0, VK_FORMAT_R32G32_SFLOAT, sizeof(ImDrawVert::pos) },
			{ 2, 0, VK_FORMAT_R8G8B8A8_UNORM, sizeof(ImDrawVert::pos) + sizeof(ImDrawVert::uv) }
		};
		VkPipelineVertexInputStateCreateInfo pvisci{ VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO, nullptr, 0, 1, &vibd, CountOf(viad), viad };
		VkPipelineInputAssemblyStateCreateInfo piasci{ VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO, nullptr, 0, VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST };
		VkPipelineTessellationStateCreateInfo ptsci{ VK_STRUCTURE_TYPE_PIPELINE_TESSELLATION_STATE_CREATE_INFO };
		VkPipelineViewportStateCreateInfo pvsci{ VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO, nullptr, 0, 1, nullptr, 1, nullptr };
		VkPipelineRasterizationStateCreateInfo prsci
		{ 
			VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO, nullptr, 0, VK_FALSE, VK_FALSE, VK_POLYGON_MODE_FILL, 
			VK_CULL_MODE_NONE, VK_FRONT_FACE_COUNTER_CLOCKWISE, VK_FALSE, 0.f, 0.f, 0.f, 1.f 
		};
		VkPipelineMultisampleStateCreateInfo pmsci{ VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO, nullptr, 0, VK_SAMPLE_COUNT_1_BIT };
		VkPipelineDepthStencilStateCreateInfo pdssci{ VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO };
		VkPipelineColorBlendAttachmentState pcbas
		{ 
			VK_TRUE, VK_BLEND_FACTOR_SRC_ALPHA, VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA, VK_BLEND_OP_ADD, 
			VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA, VK_BLEND_FACTOR_ZERO, VK_BLEND_OP_ADD, 
			VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT 
		};
		VkPipelineColorBlendStateCreateInfo pcbsci{ VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO, nullptr, 0, VK_FALSE, VK_LOGIC_OP_NO_OP, 1, &pcbas };
		VkDynamicState ds[]{ VK_DYNAMIC_STATE_VIEWPORT, VK_DYNAMIC_STATE_SCISSOR };
		VkPipelineDynamicStateCreateInfo pdsci{ VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO, nullptr, 0, CountOf(ds), ds };
		VkPipelineRenderingCreateInfo prci{ VK_STRUCTURE_TYPE_PIPELINE_RENDERING_CREATE_INFO, nullptr, 0, 1, &m_outputFormat };
		VkGraphicsPipelineCreateInfo gpci
		{
			VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO, &prci, 0, CountOf(pssci), pssci, &pvisci, &piasci, &ptsci, &pvsci, &prsci, &pmsci, &pdssci, &pcbsci, &pdsci, 
			m_bindingTable.layout()
		};
		BreakIfFailed(vkCreateGraphicsPipelines(m_vk.device(), VK_NULL_HANDLE, 1, &gpci, m_vk.alloc(), &m_pipeline));
	}
	return m_pipeline;
}
