#pragma once

struct lua_State;

namespace sandbox
{

class Scene;

class LuaState
{
public:
	explicit LuaState(Scene& scene, const char* fileName);
	void initialize();
	void finalize();
private:
	void callMain(int idx);
	lua_State* m_lua;
};

}
