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

int getgc(lua_State* L) {
    bool IncludeTables = lua_gettop(L) ? luaL_optboolean(L, 1, 0) : false;

    lua_newtable(L);

    struct CGarbageCollector
    {
        lua_State* L;
        int IncludeTables;
        int Count;
    } Gct{ L, IncludeTables, 0 };

    luaM_visitgco(L, &Gct, [](void* Context, lua_Page* Page, GCObject* Gco) {
        auto Gct = static_cast<CGarbageCollector*>(Context);

        if (!((Gco->gch.marked ^ WHITEBITS) & otherwhite(Gct->L->global)))
            return false;

        auto tt = Gco->gch.tt;
        if (tt == LUA_TFUNCTION || tt == LUA_TUSERDATA || (Gct->IncludeTables && tt == LUA_TTABLE))
        {
            Gct->L->top->value.gc = Gco;
            Gct->L->top->tt = Gco->gch.tt;
            Gct->L->top++;

            lua_rawseti(Gct->L, -2, ++Gct->Count);
        }
        return false;
    });

    return 1;
}

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
