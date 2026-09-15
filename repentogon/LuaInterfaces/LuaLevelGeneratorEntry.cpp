#include "IsaacRepentance.h"
#include "LuaCore.h"
#include "HookSystem.h"

LUA_FUNCTION(Lua_RoomPlacerSetColIdx) {
	LevelGenerator_Room* room = lua::GetRawUserdata<LevelGenerator_Room*>(L, 1, lua::metatables::LevelGeneratorEntryMT);
	room->_gridColIdx = (unsigned int)luaL_checkinteger(L, 2);
	return 0;
}

LUA_FUNCTION(Lua_RoomPlacerSetLineIdx) {
	LevelGenerator_Room* room = lua::GetRawUserdata<LevelGenerator_Room*>(L, 1, lua::metatables::LevelGeneratorEntryMT);
	room->_gridLineIdx = (unsigned int)luaL_checkinteger(L, 2);
	return 0;
}

LUA_FUNCTION(Lua_RoomPlacerSetAllowedDoors) {
	LevelGenerator_Room* room = lua::GetRawUserdata<LevelGenerator_Room*>(L, 1, lua::metatables::LevelGeneratorEntryMT);
	room->_doors = (unsigned int)luaL_checkinteger(L, 2);
	return 0;
}

LUA_FUNCTION(Lua_LevelGeneratorEntry) {
	LevelGenerator_Room* ud = (LevelGenerator_Room*)lua_newuserdata(L, sizeof(LevelGenerator_Room));
	luaL_setmetatable(L, lua::metatables::LevelGeneratorEntryMT);
	return 1;
}

void RegisterRoomPlacer(lua_State* L) {
	lua::RegisterGlobalClassFunction(L, lua::GlobalClasses::Isaac, "LevelGeneratorEntry", Lua_LevelGeneratorEntry);

	luaL_Reg functions[] = {
		{ "SetColIdx", Lua_RoomPlacerSetColIdx },
		{ "SetLineIdx", Lua_RoomPlacerSetLineIdx },
		{ "SetAllowedDoors", Lua_RoomPlacerSetAllowedDoors },
		{ NULL, NULL }
	};
	lua::RegisterNewClass(L, lua::metatables::LevelGeneratorEntryMT, lua::metatables::LevelGeneratorEntryMT, functions);
}

HOOK_METHOD(LuaEngine, RegisterClasses, () -> void) {
	super();

	lua::LuaStackProtector protector(_state);
	RegisterRoomPlacer(_state);
}