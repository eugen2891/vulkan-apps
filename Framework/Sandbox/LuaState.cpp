#include "LuaState.hpp"

#include "Scene.hpp"

extern "C"
{
#include "../Lua/lua.h"
#include "../Lua/lualib.h"
#include "../Lua/lauxlib.h"
}

#include <glm/trigonometric.hpp>

namespace sandbox
{
namespace lua
{

static const char* MainVar = "__main__";
static const char* SceneVar = "__scene__";

static Scene& GetScene(lua_State* L)
{
	lua_getglobal(L, SceneVar);
	Scene& retval = *reinterpret_cast<Scene*>(lua_touserdata(L, -1));
	lua_pop(L, 1);
	return retval;
}

static float Float(lua_State* L, int idx)
{
	float retval = float(lua_tonumber(L, idx));
	return retval;
}

static glm::vec3 Float3(lua_State* L, int idx)
{
	glm::vec3 retval{};
	if (lua_type(L, idx) == LUA_TTABLE)
	{
		lua_rawgeti(L, idx, 1);
		retval.x = float(lua_tonumber(L, -1));
		lua_rawgeti(L, idx, 2);
		retval.y = float(lua_tonumber(L, -1));
		lua_rawgeti(L, idx, 3);
		retval.z = float(lua_tonumber(L, -1));
		lua_pop(L, 3);
	}
	return retval;
}

}

static const luaL_Reg LuaFunctions[]
{
	{
		"perspective", [](lua_State* L)
		{
			float fov = lua::Float(L, 4);
			glm::vec3 from = lua::Float3(L, 1), to = lua::Float3(L, 2), up = lua::Float3(L, 3);
			lua::GetScene(L).perspective(from, to, up, fov);
			return 0; 
		}
	},
	{
		"point_light", [](lua_State* L)
		{
			int nArgs = lua_gettop(L);			
			glm::vec3 position = lua::Float3(L, 1);
			glm::vec4 color(nArgs > 1 ? lua::Float3(L, 2) : glm::vec3{ 1.f, 1.f, 1.f }, 1.f);
			glm::vec3 attenuation = nArgs > 2 ? lua::Float3(L, 3) : glm::vec3{ 1.f, 0.f, 0.f };
			lua::GetScene(L).pointLight(position, color, attenuation);
			return 0; 
		}
	},
	{
		"quad_mesh", [](lua_State* L)
		{
			int nArgs = lua_gettop(L);
			glm::vec4 albedoColor = nArgs > 0 ? glm::vec4{ lua::Float3(L, 1), 1.f } : glm::vec4{ 1.f, 1.f, 1.f, 1.f };
			lua::GetScene(L).mesh(albedoColor, eMesh_Quad);
			return 0; 
		}
	},
	{
		"push_transform", [](lua_State* L)
		{
			lua::GetScene(L).pushTransform();
			return 0; 
		}
	},
	{
		"pop_transform", [](lua_State* L)
		{
			lua::GetScene(L).popTransform();
			return 0; 
		}
	},
	{
		"translate", [](lua_State* L) 
		{
			glm::vec3 offs = lua::Float3(L, 1);
			lua::GetScene(L).translate(offs);
			return 0; 
		}
	},
	{
		"rotate", [](lua_State* L) 
		{
			float angle = lua::Float(L, 2);
			glm::vec3 axis = lua::Float3(L, 1);
			lua::GetScene(L).rotate(angle, axis);
			return 0; 
		}
	},
	{
		"scale", [](lua_State* L) 
		{
			glm::vec3 ratios = lua::Float3(L, 1);
			lua::GetScene(L).scale(ratios);
			return 0;
		}
	},
	{
		"math_ctg", [](lua_State* L)
		{
			const double angle = glm::radians(lua_tonumber(L, 1));
			lua_pushnumber(L, 1.0 / glm::tan(angle));
			return 1;
		}
	}
};

LuaState::LuaState(Scene& scene,const char* fileName)
{
	lua_State* L = luaL_newstate();
	if (L)
	{
		lua_pushlightuserdata(L, &scene);
		lua_setglobal(L, lua::SceneVar);
		for (auto const& fn : LuaFunctions) lua_register(L, fn.name, fn.func);
		if (fileName)
		{
			if (luaL_dofile(L, fileName) != LUA_OK)
			{
				const char* msg = lua_tostring(L, -1);
				printf("%s\n", msg);
			}
		}
	}
	m_lua = L;
}

void LuaState::initialize()
{
	callMain(1);
}

void LuaState::finalize()
{
	callMain(3);
}

void LuaState::callMain(int idx)
{
	if (lua_getglobal(m_lua, lua::MainVar) == LUA_TTABLE)
	{
		if (lua_rawgeti(m_lua, -1, idx) == LUA_TFUNCTION)
		{
			lua_pcall(m_lua, 0, 0, 0);
		}
	}
	lua_pop(m_lua, 1);
}

}