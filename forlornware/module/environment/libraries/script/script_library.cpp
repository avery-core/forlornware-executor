#include "script_library.hpp"

int identifyexecutor(lua_State* L)
{
	lua_pushstring(L, "ForlornWare");
	lua_pushstring(L, "1.0.0");
	return 2;
}

int getgenv(lua_State* L)
{
	lua_pushvalue(L, LUA_ENVIRONINDEX);
	return 1;
}

std::string read_bytecode(uintptr_t addr) {
	auto str = addr + 0x10;
	auto len = *(size_t*)(str + 0x10);
	auto data = *(size_t*)(str + 0x18) > 0xf ? *(uintptr_t*) (str) : str;
	return std::string((char*)(data), len);
}

int getscriptbytecode(lua_State* L) {
	luaL_checktype(L, 1, LUA_TUSERDATA);
	uintptr_t code[4] = {};
	uintptr_t script_pointer = *(uintptr_t*)lua_topointer(L, 1);
	roblox::request_code((uintptr_t)code, script_pointer);
	auto decompressed = global_functions::decompress_bytecode(read_bytecode(code[1]));
	lua_pushlstring(L, decompressed.data(), decompressed.size());
	return 1;
}

int getgc(lua_State* L) { lua_newtable(L); int idx = 1; global_State* g = G(L); for (GCObject* o = g->rootgc; o != NULL; o = o->next) { if (o->tt == LUA_TTABLE || o->tt == LUA_TFUNCTION) { lua_pushinteger(L, idx++); if (o->tt == LUA_TTABLE) { sethvalue(L, L->top, gco2h(o)); } else { setclvalue(L, L->top, gco2cl(o)); } api_incr_top(L); lua_settable(L, -3); } } return 1; }

void script_library::initialize(lua_State* L)
{
	register_env_functions(L,
		{
			{"identifyexecutor", identifyexecutor},
			{"getgenv", getgenv},
			{"getscriptbytecode", getscriptbytecode},
			{"getgc", getgc},
			{nullptr, nullptr}
		});
}
