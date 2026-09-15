#include "IsaacRepentance.h"
#include "LuaCore.h"
#include "HookSystem.h"

#include "Level.h"
#include "LuaEntitySaveState.h"
#include "Room/RoomPlacement.h"
#include "../SaveStateManagement/EntitySaveStateManagement.h"
#include "../Utils/LevelGenUtils.h"
#include <unordered_set>

LevelASM levelASM;

LUA_FUNCTION(Lua_LevelCanSpawnDoorOutline) {
	Level* level = lua::GetLuabridgeUserdata<Level*>(L, 1, lua::Metatables::LEVEL, "Level");
	int roomIDX = (int)luaL_checkinteger(L, 2);
	unsigned int doorSlot = (unsigned int)luaL_checkinteger(L, 3);
	lua_pushboolean(L, level->CanSpawnDoorOutline(roomIDX, doorSlot));
	return 1;
}

LUA_FUNCTION(Lua_LevelHasAbandonedMineshaft)
{
	Level* level = lua::GetLuabridgeUserdata<Level*>(L, 1, lua::Metatables::LEVEL, "Level");
	lua_pushboolean(L, level->HasAbandonedMineshaft());

	return 1;
}

LUA_FUNCTION(Lua_LevelHasMirrorDimension)
{
	Level* level = lua::GetLuabridgeUserdata<Level*>(L, 1, lua::Metatables::LEVEL, "Level");
	lua_pushboolean(L, level->HasMirrorDimension());

	return 1;
}

LUA_FUNCTION(Lua_LevelHasPhotoDoor)
{
	Level* level = lua::GetLuabridgeUserdata<Level*>(L, 1, lua::Metatables::LEVEL, "Level");
	lua_pushboolean(L, level->HasPhotoDoor());

	return 1;
}

static std::string CustomStageName;

LUA_FUNCTION(lua_LevelSetName) {
	Level* level = lua::GetLuabridgeUserdata<Level*>(L, 1, lua::Metatables::LEVEL, "Level");
	const char* name = luaL_checkstring(L, 2);
	CustomStageName = name;
	return 0;
}

LUA_FUNCTION(lua_LevelSetGreedWavesClearedWithoutRedHeartDamage) {
	Level* level = lua::GetLuabridgeUserdata<Level*>(L, 1, lua::Metatables::LEVEL, "Level");
	level->_greedwavesclearedwithoutredheartdamage = (uint32_t)luaL_checkinteger(L, 2);
	return 0;
}
LUA_FUNCTION(lua_LevelGetGreedWavesClearedWithoutRedHeartDamage) {
	Level* level = lua::GetLuabridgeUserdata<Level*>(L, 1, lua::Metatables::LEVEL, "Level");
	lua_pushinteger(L, level->_greedwavesclearedwithoutredheartdamage);
	return 1;
}

LUA_FUNCTION(lua_LevelIsStageAvailable) {
	Level* level = lua::GetLuabridgeUserdata<Level*>(L, 1, lua::Metatables::LEVEL, "Level");
	int levelStage = (int)luaL_checkinteger(L, 2);
	int stageType = (int)luaL_checkinteger(L, 3);
	lua_pushboolean(L, level->IsStageAvailable(levelStage, stageType));
	return 1;
}

LUA_FUNCTION(Lua_GetDimension) {
	Game* level = lua::GetLuabridgeUserdata<Game*>(L, 1, lua::Metatables::LEVEL, "Level");
	lua_pushinteger(L, level->GetDimension());
	return 1;
}

LUA_FUNCTION(Lua_GetForceSpecialQuest) {
	lua_pushinteger(L, (int)levelASM.ForceSpecialQuest);
	return 1;
}

LUA_FUNCTION(Lua_SetForceSpecialQuest) {
	levelASM.ForceSpecialQuest = (int)luaL_checkinteger(L, 2);
	return 0;
}

LUA_FUNCTION(Lua_GetMyosotisPickups) {
	Game* level = lua::GetLuabridgeUserdata<Game*>(L, 1, lua::Metatables::LEVEL, "Level");
	Lua_EntitiesSaveStateVector* ud = lua::place<Lua_EntitiesSaveStateVector>(L, lua::metatables::EntitiesSaveStateVectorMT);
	ud->data = (&level->_myosotisPickups);
	return 1;
}

LUA_FUNCTION(Lua_LevelIsAltPath) {
	bool ret = false;
	// If ForceSpecialQuest is -1, quest doors are disabled
	if (levelASM.ForceSpecialQuest > -1) {
		// ForceSpecialQuest 0 defaults to vanilla behavior
		ret = levelASM.ForceSpecialQuest > 0 || (g_Game->_stageType == 4 || g_Game->_stageType == 5);
	}
	lua_pushboolean(L, ret);
	return 1;
}

/*
HOOK_METHOD(Level, IsAltPath, () -> bool) {
	bool ret = false;
	// If ForceSpecialQuest is -1, quest doors are disabled
	if (levelASM.ForceSpecialQuest > -1) {
		// ForceSpecialQuest 0 defaults to vanilla behavior
		ret = levelASM.ForceSpecialQuest > 0 || (g_Game->_stageType == 4 || g_Game->_stageType == 5);
	}
	return ret;
}
*/

bool CheckQuest(Level* level, const int levelStage, const int expected, const int quest) {
	unsigned int stage;
	if (g_Game->_difficulty < 2 && levelASM.ForceSpecialQuest > -1) {
		if (stage = g_Game->_stage, 5 < stage - 1 || (g_Game->_levelStateFlags & 0x10000) == 0 && level->IsAltPath()) {
			return (levelASM.ForceSpecialQuest == quest || levelStage == expected || (levelStage == expected - 1 && (g_Game->_curses & 2) != 0));
		}
	}
	return false;
}

HOOK_METHOD(Level, HasMirrorDimension, () -> bool) {
	return CheckQuest(this, g_Game->_stage, 2, 1);
}

HOOK_METHOD(Level, HasAbandonedMineshaft, () -> bool) {
	return CheckQuest(this, g_Game->_stage, 4, 2);
}

/* HOOK_METHOD(Level, GetName, () -> std::string) {
	std::string name = super();
	if (!CustomStageName.empty()) {
		name = CustomStageName;
	}

	return name;
} */

HOOK_GLOBAL(GetLevelName, (std_string* result, uint32_t levelStage, uint32_t stageType, uint32_t curseMask, uint32_t unk1, uint32_t unk2, bool unk3) -> void, __cdecl) {
	super(result, levelStage, stageType, curseMask, unk1, unk2, unk3);
	if (!CustomStageName.empty()) {
		*result = CustomStageName;
	}
}

LUALIB_API int LuaCheckDimension(lua_State* L, int arg) {
	lua_Integer dimension = luaL_optinteger(L, arg, -1);
	if (dimension < -1 || dimension > 2) {
		return luaL_argerror(L, arg, "Invalid Dimension");
	}
	return (int)dimension;
}

// Generates a determinstic seed for room placement based on the shape of the room and where we are trying to place it.
// This is very similar to what is done when the game creates red rooms.
uint32_t GetRoomPlacementSeed(const int roomShape, const int x, const int y) {
	const uint32_t seed = (g_Game->_dungeonPlacementSeed + y * 929) * (x + roomShape);
	RNG rng;
	rng.SetSeed(seed, 35);
	return std::max(rng.Next(), 1u);
}

// Validates room placement seed passed from lua. If 0 or nil is passed, generate a seed.
LUALIB_API uint32_t LuaCheckRoomPlacementSeed(lua_State* L, int arg, const int roomShape, const int x, const int y) {
	lua_Integer inputSeed = luaL_optinteger(L, arg, 0);
	if (inputSeed < 0) {
		luaL_argerror(L, arg, "Invalid Seed");
	} else if (inputSeed == 0) {
		return GetRoomPlacementSeed(roomShape, x, y);
	}
	return (uint32_t)inputSeed;
}

LUA_FUNCTION(Lua_LevelCanPlaceRoom) {
	int stackIdx = 2;

	int roomShape = 0;
	int doorMask = 0;

	if (lua_type(L, stackIdx) == LUA_TUSERDATA) {
		RoomConfig_Room* roomConfig = lua::GetLuabridgeUserdata<RoomConfig_Room*>(L, stackIdx++, lua::Metatables::CONST_ROOM_CONFIG_ROOM, "RoomConfig");
		if (roomConfig) {
			roomShape = roomConfig->Shape;
			doorMask = roomConfig->Doors;
		}
	} else {
		roomShape = (int)luaL_checkinteger(L, stackIdx++);
		doorMask = (int)luaL_optinteger(L, stackIdx++, -1);
	}

	const int gridIndex = (int)luaL_checkinteger(L, stackIdx++);
	if (gridIndex < 0 || gridIndex > 168) {
		lua_pushboolean(L, false);
		return 1;
	}
	const XY coords = RoomIndexToCoords(gridIndex);
	const int dimension = LuaCheckDimension(L, stackIdx++);
	const bool allowMultipleDoors = lua::luaL_optboolean(L, stackIdx++, true);
	const bool allowSpecialNeighbors = lua::luaL_optboolean(L, stackIdx++, false);
	const bool allowNoNeighbors = lua::luaL_optboolean(L, stackIdx++, false);
	const bool result = CanPlaceRoom(roomShape, doorMask, coords.x, coords.y, dimension, allowMultipleDoors, allowSpecialNeighbors, allowNoNeighbors);
	lua_pushboolean(L, result);
	return 1;
}

LUA_FUNCTION(Lua_LevelTryPlaceRoom) {
	RoomConfig_Room* roomConfig = lua::GetLuabridgeUserdata<RoomConfig_Room*>(L, 2, lua::Metatables::CONST_ROOM_CONFIG_ROOM, "RoomConfig");
	const int gridIndex = (int)luaL_checkinteger(L, 3);
	if (gridIndex < 0 || gridIndex > 168) {
		lua_pushboolean(L, false);
		return 1;
	}
	const XY coords = RoomIndexToCoords(gridIndex);
	const int dimension = LuaCheckDimension(L, 4);
	const uint32_t seed = LuaCheckRoomPlacementSeed(L, 5, roomConfig->Shape, coords.x, coords.y);
	const bool allowMultipleDoors = lua::luaL_optboolean(L, 6, true);
	const bool allowSpecialNeighbors = lua::luaL_optboolean(L, 7, false);
	const bool allowNoNeighbors = lua::luaL_optboolean(L, 8, false);
	RoomDescriptor* newRoom = TryPlaceRoom(roomConfig, coords.x, coords.y, dimension, seed, allowMultipleDoors, allowSpecialNeighbors, allowNoNeighbors);
	if (newRoom) {
		lua::luabridge::UserdataPtr::push(L, newRoom, lua::GetMetatableKey(lua::Metatables::ROOM_DESCRIPTOR));
	} else {
		lua_pushnil(L);
	}
	return 1;
}

LUA_FUNCTION(Lua_LevelCanPlaceRoomAtDoor) {
	int stackIdx = 2;

	int roomShape = 0;
	int doorMask = 0;

	if (lua_type(L, stackIdx) == LUA_TUSERDATA) {
		RoomConfig_Room* roomConfig = lua::GetLuabridgeUserdata<RoomConfig_Room*>(L, stackIdx++, lua::Metatables::CONST_ROOM_CONFIG_ROOM, "RoomConfig");
		if (roomConfig) {
			roomShape = roomConfig->Shape;
			doorMask = roomConfig->Doors;
		}
	} else {
		roomShape = (int)luaL_checkinteger(L, stackIdx++);
		doorMask = (int)luaL_optinteger(L, stackIdx++, -1);
	}

	RoomDescriptor* roomDescToConnect = lua::GetLuabridgeUserdata<RoomDescriptor*>(L, stackIdx++, lua::Metatables::ROOM_DESCRIPTOR, "RoomDescriptor");
	const int doorSlot = (int)luaL_checkinteger(L, stackIdx++);
	const bool allowMultipleDoors = lua::luaL_optboolean(L, stackIdx++, true);
	const bool allowSpecialNeighbors = lua::luaL_optboolean(L, stackIdx++, false);
	const bool result = CanPlaceRoomAtDoor(roomShape, doorMask, roomDescToConnect, doorSlot, allowMultipleDoors, allowSpecialNeighbors);
	lua_pushboolean(L, result);
	return 1;
}

LUA_FUNCTION(Lua_LevelTryPlaceRoomAtDoor) {
	RoomConfig_Room* roomConfigToPlace = lua::GetLuabridgeUserdata<RoomConfig_Room*>(L, 2, lua::Metatables::CONST_ROOM_CONFIG_ROOM, "RoomConfig");
	RoomDescriptor* roomDescToConnect = lua::GetLuabridgeUserdata<RoomDescriptor*>(L, 3, lua::Metatables::ROOM_DESCRIPTOR, "RoomDescriptor");
	if (!roomDescToConnect || !roomDescToConnect->Data) {
		lua_pushboolean(L, false);
		return 1;
	}
	const int doorSlot = (int)luaL_checkinteger(L, 4);

	// Find the target coordinates of this door, as we might use it to generate a seed.
	const DoorSourceTarget doorsourceTarget = GetDoorSourceTarget(roomDescToConnect->GridIndex, roomDescToConnect->Data->Shape, doorSlot, false);
	if (!doorsourceTarget.IsValid()) {
		lua_pushboolean(L, false);
		return 1;
	}
	const uint32_t seed = LuaCheckRoomPlacementSeed(L, 5, roomConfigToPlace->Shape, doorsourceTarget.target.x, doorsourceTarget.target.y);

	const bool allowMultipleDoors = lua::luaL_optboolean(L, 6, true);
	const bool allowSpecialNeighbors = lua::luaL_optboolean(L, 7, false);
	RoomDescriptor* newRoom = TryPlaceRoomAtDoor(roomConfigToPlace, roomDescToConnect, doorSlot, seed, allowMultipleDoors, allowSpecialNeighbors);
	if (newRoom) {
		lua::luabridge::UserdataPtr::push(L, newRoom, lua::GetMetatableKey(lua::Metatables::ROOM_DESCRIPTOR));
	} else {
		lua_pushnil(L);
	}
	return 1;
}

LUA_FUNCTION(Lua_LevelFindValidRoomPlacementLocations) {
	int stackIdx = 2;

	int roomShape = ROOMSHAPE_1x1;
	int doorMask = -1;

	if (lua_type(L, stackIdx) == LUA_TUSERDATA) {
		RoomConfig_Room* roomConfig = lua::GetLuabridgeUserdata<RoomConfig_Room*>(L, stackIdx++, lua::Metatables::CONST_ROOM_CONFIG_ROOM, "RoomConfig");
		if (roomConfig) {
			roomShape = roomConfig->Shape;
			doorMask = roomConfig->Doors;
		}
	} else {
		roomShape = (int)luaL_optinteger(L, stackIdx++, roomShape);
		doorMask = (int)luaL_optinteger(L, stackIdx++, doorMask);
	}

	const int dimension = LuaCheckDimension(L, stackIdx++);
	const bool allowMultipleDoors = lua::luaL_optboolean(L, stackIdx++, true);
	const bool allowSpecialNeighbors = lua::luaL_optboolean(L, stackIdx++, false);
	const std::set<int> validLocations = FindValidRoomPlacementLocations(roomShape, doorMask, dimension, allowMultipleDoors, allowSpecialNeighbors);
	
	lua_newtable(L);
	int i = 0;
	for (const int gridIndex : validLocations) {
		lua_pushinteger(L, gridIndex);
		lua_rawseti(L, -2, i + 1);
		i++;
	}

	return 1;
}

LUA_FUNCTION(Lua_LevelGetNeighboringRooms) {
	const int gridIndex = (int)luaL_checkinteger(L, 2);
	const int roomShape = (int)luaL_checkinteger(L, 3);
	const int dimension = LuaCheckDimension(L, 4);
	
	const std::map<int, RoomDescriptor*> neighbors = GetNeighboringRooms(gridIndex, roomShape, dimension);

	lua_newtable(L);
	for (const auto& [doorSlot, neighborDesc] : neighbors) {
		lua::luabridge::UserdataPtr::push(L, neighborDesc, lua::GetMetatableKey(lua::Metatables::ROOM_DESCRIPTOR));
		lua_rawseti(L, -2, doorSlot);
	}

	return 1;
}

LUA_FUNCTION(Lua_LevelGetGenerationRNG) {
	lua::luabridge::UserdataPtr::push(L, &g_Game->_generationRNG, lua::Metatables::RNG);

	return 1;
}

extern bool g_CustomDungeonGenerated;

LUA_FUNCTION(Lua_LevelUpdateDoors) {
	Level* level = lua::GetLuabridgeUserdata<Level*>(L, 1, lua::Metatables::LEVEL, "Level");
	bool prev = g_CustomDungeonGenerated;
	g_CustomDungeonGenerated = false;
	level->update_doors();
	g_CustomDungeonGenerated = prev;
	g_Game->UpdateVisibility();
	return 0;
}


static void ClearRoomDescriptor(RoomDescriptor& desc, int listIndex) {
	desc.Reset();
	desc.GridIndex = -1;
	desc.SafeGridIndex = -1;
	desc.ListIndex = listIndex;
	desc.Dimension = 0;
	desc.Data = nullptr;
	desc.OverrideData = nullptr;
	desc.AllowedDoors = 0;
	for (int i = 0; i < 8; i++) {
		desc.Doors[i] = -1;
	}
	desc.DisplayFlags = 0;
	desc.VisitedCount = 0;
	desc.Flags = 0;
	desc.TotalMoneySpent = 0;
	desc.ClearCount = 0;
	desc.GreedSubType = 0;
	desc.PoopCount = 0;
	desc.TKeeperCoinSpawns = 0;
	desc.PitsCount = 0;
	desc.DecorationSeed = 0;
	desc.SpawnSeed = 0;
	desc.AwardSeed = 0;
	desc.BossDeathSeed = 0;
	desc.ShopItemIdx = -1;
	desc.ShopItemDiscountIdx = -1;
	desc.DeliriumDistance = 0;
	desc.Group = 0;
}

static bool IsValidStage(int stage) {
	return stage >= 0 && (stage < STB_UNUSED1 || stage > STB_ULTRA_GREED) && stage != STB_THE_VOID && stage < NUM_STB;
}

static bool RoomOccupiesGridIndex(RoomDescriptor* desc, int targetGridIdx) {
	if (!desc || !desc->Data || targetGridIdx < 0 || targetGridIdx >= 169) return false;
	XY baseCoords(desc->GridIndex % 13, desc->GridIndex / 13);
	std::vector<XY> occupied = GetOccupiedCoords(baseCoords, desc->Data->Shape);
	for (const auto& xy : occupied) {
		if (xy.ToGridIdx() == targetGridIdx) {
			return true;
		}
	}
	return false;
}

static bool InternalRemoveRoom(Level* level, RoomDescriptor* desc) {
	if (!desc || !desc->Data) return false;

	RoomDescriptor* currentRoomDesc = g_Game->GetCurrentRoomDesc();
	Room* curRoom = g_Game->GetCurrentRoom();
	RoomDescriptor* activeRoomDesc = curRoom ? curRoom->_descriptor : (g_Game->_room ? g_Game->_room->_descriptor : nullptr);
	if (desc == currentRoomDesc || desc == activeRoomDesc || RoomOccupiesGridIndex(desc, (int)g_Game->_currentRoomIdx)) {
		return false;
	}
	if (g_Game->GetRoomTransition()->GetTransitionMode() != 0 && RoomOccupiesGridIndex(desc, (int)g_Game->_lastRoomIdx)) {
		return false;
	}
	if (g_Game->IsGreedMode() && g_Game->_startingRoomIdx < 169 && 
		(desc->SafeGridIndex == (int)g_Game->_startingRoomIdx || desc->GridIndex == (int)g_Game->_startingRoomIdx || RoomOccupiesGridIndex(desc, (int)g_Game->_startingRoomIdx))) {
		return false;
	}

	int dimension = (desc->Dimension >= 0) ? desc->Dimension : (int)g_Game->_currentDimensionIdx;
	XY baseCoords(desc->GridIndex % 13, desc->GridIndex / 13);
	std::vector<XY> occupied = GetOccupiedCoords(baseCoords, desc->Data->Shape);
	std::unordered_set<int> occupiedIndices;
	for (const auto& xy : occupied) {
		occupiedIndices.insert(xy.ToGridIdx());
	}

	for (int s = 0; s < 8; s++) {
		if (desc->Doors[s] >= 0) {
			RoomDescriptor* neighbor = g_Game->GetRoomByIdx(desc->Doors[s], dimension);
			if (neighbor && neighbor->Data) {
				for (int ns = 0; ns < 8; ns++) {
					if (neighbor->Doors[ns] >= 0 && occupiedIndices.count(neighbor->Doors[ns])) {
						neighbor->Doors[ns] = -1;
						neighbor->AllowedDoors &= ~(1 << ns);
					}
				}
			}
		}
	}

	for (const auto& xy : occupied) {
		const int dx[] = { 0, 1, 0, -1 };
		const int dy[] = { -1, 0, 1, 0 };
		for (int d = 0; d < 4; d++) {
			int nx = xy.x + dx[d];
			int ny = xy.y + dy[d];
			if (nx >= 0 && nx < 13 && ny >= 0 && ny < 13) {
				int nIdx = ny * 13 + nx;
				if (!occupiedIndices.count(nIdx)) {
					RoomDescriptor* neighbor = g_Game->GetRoomByIdx(nIdx, dimension);
					if (neighbor && neighbor->Data) {
						for (int ns = 0; ns < 8; ns++) {
							if (neighbor->Doors[ns] >= 0 && occupiedIndices.count(neighbor->Doors[ns])) {
								neighbor->Doors[ns] = -1;
								neighbor->AllowedDoors &= ~(1 << ns);
							}
						}
					}
				}
			}
		}
	}

	if (curRoom) {
		for (int ns = 0; ns < 8; ns++) {
			GridEntity_Door* door = curRoom->GetDoor(ns);
			if (door && (occupiedIndices.count(door->_targetRoomIdx) || (currentRoomDesc && currentRoomDesc->Doors[ns] < 0))) {
				curRoom->RemoveGridEntityImmediate(door->_gridIndex, 0x3E8 /*wall*/, false);
				curRoom->_doors[ns] = nullptr;
				Entity_Effect* outline = curRoom->_redDoorOutlines[ns];
				if (outline) {
					outline->Remove();
					curRoom->_redDoorOutlines[ns] = nullptr;
				}
			}
		}
	}

	EntitySaveStateManagement::EntitySaveState_ClearBatch(desc->SavedEntities);
	EntitySaveStateManagement::EntitySaveState_ClearBatch(desc->SavedEffects);

	for (const auto& xy : occupied) {
		int offsetIdx = dimension * 169 + xy.ToGridIdx();
		if (offsetIdx >= 0 && offsetIdx < 507) {
			g_Game->_roomOffset[offsetIdx] = -1;
		}
	}

	int listIdx = desc->ListIndex;
	int lastIdx = (int)g_Game->_nbRooms - 1;

	if (listIdx >= 0 && listIdx < lastIdx) {
		RoomDescriptor& lastRoom = g_Game->_gridRooms[lastIdx];
		desc->Reset();
		*desc = std::move(lastRoom);
		desc->ListIndex = listIdx;

		if (curRoom && curRoom->_descriptor == &lastRoom) {
			curRoom->_descriptor = desc;
		}
		if (g_Game->_room && g_Game->_room->_descriptor == &lastRoom) {
			g_Game->_room->_descriptor = desc;
		}

		int lastDim = (desc->Dimension >= 0) ? desc->Dimension : 0;
		XY lastBaseCoords(desc->GridIndex % 13, desc->GridIndex / 13);
		std::vector<XY> lastOccupied = GetOccupiedCoords(lastBaseCoords, desc->Data->Shape);
		for (const auto& xy : lastOccupied) {
			int offsetIdx = lastDim * 169 + xy.ToGridIdx();
			if (offsetIdx >= 0 && offsetIdx < 507) {
				g_Game->_roomOffset[offsetIdx] = listIdx;
			}
		}

		if ((int)g_Game->_lastBossRoomListIdx == lastIdx) {
			g_Game->_lastBossRoomListIdx = listIdx;
		}

		ClearRoomDescriptor(lastRoom, lastIdx);
	} else {
		ClearRoomDescriptor(*desc, listIdx);
	}

	if (listIdx == (int)g_Game->_lastBossRoomListIdx) {
		g_Game->_lastBossRoomListIdx = -1;
	}

	if (g_Game->_nbRooms > 0) {
		g_Game->_nbRooms--;
	}

	g_Game->UpdateVisibility();
	Minimap* minimap = g_Game->GetMinimap();
	if (minimap) minimap->Refresh();

	return true;
}

static bool PrepareTargetLocation(Level* level, int targetCol, int targetRow, int shape, int dimension) {
	int targetIdx = targetRow * 13 + targetCol;
	XY targetBase(targetCol, targetRow);
	std::vector<XY> newOccupied = GetOccupiedCoords(targetBase, shape);
	for (const auto& xy : newOccupied) {
		if (xy.ToGridIdx() < 0) {
			return false;
		}
	}

	if (g_Game->IsGreedMode() && g_Game->_startingRoomIdx < 169) {
		for (const auto& xy : newOccupied) {
			if (xy.ToGridIdx() == (int)g_Game->_startingRoomIdx) {
				return false;
			}
		}
	}

	RoomDescriptor* currentRoomDesc = g_Game->GetCurrentRoomDesc();
	Room* curRoom = g_Game->GetCurrentRoom();
	RoomDescriptor* activeRoomDesc = curRoom ? curRoom->_descriptor : (g_Game->_room ? g_Game->_room->_descriptor : nullptr);

	bool found = true;
	while (found) {
		found = false;
		for (const auto& xy : newOccupied) {
			int gIdx = xy.ToGridIdx();
			if (gIdx >= 0 && gIdx < 169) {
				RoomDescriptor* ex = g_Game->GetRoomByIdx(gIdx, dimension);
				if (ex && ex->Data) {
					if (ex == currentRoomDesc || ex == activeRoomDesc || RoomOccupiesGridIndex(ex, (int)g_Game->_currentRoomIdx)) {
						return false;
					}
					if (g_Game->GetRoomTransition()->GetTransitionMode() != 0 && RoomOccupiesGridIndex(ex, (int)g_Game->_lastRoomIdx)) {
						return false;
					}
					InternalRemoveRoom(level, ex);
					found = true;
					break;
				}
			}
		}
	}
	return true;
}

static int CalculateRuntimeAutoDoors(int targetIdx, int shape, int dimension) {
	int doors = 0;
	RoomDescriptor* targetDesc = g_Game->GetRoomByIdx(targetIdx, dimension);
	for (int slot = 0; slot < 8; slot++) {
		DoorSourceTarget dst = GetDoorSourceTarget(targetIdx, shape, slot, false);
		if (dst.IsValid()) {
			int neighborIdx = dst.GetTargetIdx();
			RoomDescriptor* neighborDesc = g_Game->GetRoomByIdx(neighborIdx, dimension);
			if (neighborDesc && neighborDesc->Data) {
				if (targetDesc && (neighborDesc == targetDesc || RoomOccupiesGridIndex(targetDesc, neighborIdx))) {
					continue;
				}
				for (int returnSlot : GetPossibleReturnDoorSlots(slot)) {
					DoorSourceTarget rdst = GetDoorSourceTarget(neighborDesc->GridIndex, neighborDesc->Data->Shape, returnSlot, false);
					if (rdst.ConnectsTo(dst)) {
						doors |= (1 << slot);
						break;
					}
				}
			}
		}
	}
	return doors;
}

static void ConnectRoomDoors(Level* level, RoomDescriptor* newRoomDesc, int targetIdx, int shape, int allowed_doors, bool explicit_doors, int dimension) {
	if (!newRoomDesc || !newRoomDesc->Data) return;

	newRoomDesc->SafeGridIndex = targetIdx;
	newRoomDesc->GridIndex = targetIdx;
	newRoomDesc->Dimension = dimension;

	XY baseCoords(targetIdx % 13, targetIdx / 13);
	std::vector<XY> occupied = GetOccupiedCoords(baseCoords, shape);
	for (const auto& xy : occupied) {
		int offsetIdx = dimension * 169 + xy.ToGridIdx();
		if (offsetIdx >= 0 && offsetIdx < 507) {
			g_Game->_roomOffset[offsetIdx] = newRoomDesc->ListIndex;
		}
	}

	bool auto_doors = (!explicit_doors || allowed_doors == -1);
	int effective_doors = auto_doors ? CalculateRuntimeAutoDoors(targetIdx, shape, dimension) : allowed_doors;
	newRoomDesc->AllowedDoors = 0;
	for (int s = 0; s < 8; s++) {
		newRoomDesc->Doors[s] = -1;
	}

	for (int s = 0; s < 8; s++) {
		if ((effective_doors & (1 << s)) != 0) {
			DoorSourceTarget dst = GetDoorSourceTarget(targetIdx, shape, s, false);
			if (dst.IsValid()) {
				int neighborIdx = dst.GetTargetIdx();
				RoomDescriptor* neighborDesc = g_Game->GetRoomByIdx(neighborIdx, dimension);
				if (neighborDesc && neighborDesc->Data) {
					for (int returnSlot : GetPossibleReturnDoorSlots(s)) {
						DoorSourceTarget rdst = GetDoorSourceTarget(neighborDesc->GridIndex, neighborDesc->Data->Shape, returnSlot, false);
						if (rdst.ConnectsTo(dst)) {
							newRoomDesc->Doors[s] = dst.GetTargetIdx();
							newRoomDesc->AllowedDoors |= (1 << s);

							neighborDesc->Doors[returnSlot] = dst.GetSourceIdx();
							neighborDesc->AllowedDoors |= (1 << returnSlot);

							if (neighborDesc == g_Game->GetCurrentRoomDesc()) {
								Room* curRoom = g_Game->GetCurrentRoom();
								if (curRoom) {
									curRoom->MakeDoor(returnSlot);
									GridEntity_Door* door = curRoom->GetDoor(returnSlot);
									if (door) {
										door->_targetRoomIdx = dst.GetSourceIdx();
										door->SetRoomTypes(neighborDesc->Data->Type, newRoomDesc->Data->Type);
										if (curRoom->GetFrameCount() > 0) door->play_animation();
										Entity_Effect* outline = curRoom->_redDoorOutlines[returnSlot];
										if (outline) {
											outline->Remove();
											curRoom->_redDoorOutlines[returnSlot] = nullptr;
										}
									}
								}
							}
							break;
						}
					}
				}
			}
		}
	}

	newRoomDesc->DisplayFlags |= 5;
	g_Game->UpdateVisibility();
	Minimap* minimap = g_Game->GetMinimap();
	if (minimap) minimap->Refresh();
}

LUA_FUNCTION(Lua_LevelPlaceRandomRoom) {
	Level* level = lua::GetLuabridgeUserdata<Level*>(L, 1, lua::Metatables::LEVEL, "Level");
	Game* game = (Game*)level;
	int argCount = lua_gettop(L);

	RoomDescriptor* currentDesc = g_Game->GetCurrentRoomDesc();
	int dimension = (currentDesc && currentDesc->Dimension >= 0) ? currentDesc->Dimension : (int)g_Game->_currentDimensionIdx;

	uint32_t col = 0;
	uint32_t row = 0;
	int stage = RoomConfig::GetStageID(level->_stage, level->_stageType, -1);
	bool stageSpecified = false;
	int type = 1;
	int shape = ROOMSHAPE_1x1;
	int minVariant = 0;
	int maxVariant = -1;
	int minDifficulty = 0;
	int maxDifficulty = 10;
	int subtype = -1;
	int mode = -1;
	int allowed_doors = -1;
	bool explicit_doors = false;
	bool force_doors = false;
	uint32_t seed = 0;

	bool has_grid_index = false;
	int table_idx = 0;

	if (argCount >= 3 && lua_istable(L, 3)) {
		has_grid_index = true;
		table_idx = 3;
	} else if (argCount >= 4 && lua_istable(L, 4)) {
		table_idx = 4;
	} else if (argCount == 2) {
		has_grid_index = true;
	} else if (argCount == 3 && !lua_istable(L, 3)) {
		has_grid_index = true;
	}

	if (has_grid_index) {
		uint32_t grid_index = (uint32_t)luaL_checkinteger(L, 2);
		col = grid_index % 13;
		row = grid_index / 13;
	} else {
		col = (uint32_t)luaL_checkinteger(L, 2);
		row = (uint32_t)luaL_checkinteger(L, 3);
	}

	if (table_idx > 0) {
		lua_getfield(L, table_idx, "Stage");
		if (!lua_isnil(L, -1)) {
			stage = (int)lua_tointeger(L, -1);
			stageSpecified = true;
		}
		lua_pop(L, 1);

		lua_getfield(L, table_idx, "Type");
		if (!lua_isnil(L, -1)) type = (int)lua_tointeger(L, -1);
		lua_pop(L, 1);

		lua_getfield(L, table_idx, "Shape");
		if (!lua_isnil(L, -1)) shape = (int)lua_tointeger(L, -1);
		lua_pop(L, 1);

		lua_getfield(L, table_idx, "MinVariant");
		if (!lua_isnil(L, -1)) minVariant = (int)lua_tointeger(L, -1);
		lua_pop(L, 1);

		lua_getfield(L, table_idx, "MaxVariant");
		if (!lua_isnil(L, -1)) maxVariant = (int)lua_tointeger(L, -1);
		lua_pop(L, 1);

		lua_getfield(L, table_idx, "MinDifficulty");
		if (!lua_isnil(L, -1)) minDifficulty = (int)lua_tointeger(L, -1);
		lua_pop(L, 1);

		lua_getfield(L, table_idx, "MaxDifficulty");
		if (!lua_isnil(L, -1)) maxDifficulty = (int)lua_tointeger(L, -1);
		lua_pop(L, 1);

		lua_getfield(L, table_idx, "Subtype");
		if (!lua_isnil(L, -1)) subtype = (int)lua_tointeger(L, -1);
		lua_pop(L, 1);

		lua_getfield(L, table_idx, "Mode");
		if (!lua_isnil(L, -1)) mode = (int)lua_tointeger(L, -1);
		lua_pop(L, 1);

		lua_getfield(L, table_idx, "Doors");
		if (!lua_isnil(L, -1)) {
			allowed_doors = (int)lua_tointeger(L, -1);
			explicit_doors = true;
		}
		lua_pop(L, 1);

		lua_getfield(L, table_idx, "ForceDoors");
		if (!lua_isnil(L, -1)) {
			force_doors = lua_toboolean(L, -1);
		} else {
			lua_pop(L, 1);
			lua_getfield(L, table_idx, "FullMuzzle");
			if (!lua_isnil(L, -1)) {
				force_doors = lua_toboolean(L, -1);
			} else {
				lua_pop(L, 1);
				lua_getfield(L, table_idx, "Muzzle");
				if (!lua_isnil(L, -1)) {
					force_doors = lua_toboolean(L, -1);
				}
			}
		}
		lua_pop(L, 1);

		lua_getfield(L, table_idx, "Seed");
		if (!lua_isnil(L, -1)) seed = (uint32_t)lua_tointeger(L, -1);
		lua_pop(L, 1);

		lua_getfield(L, table_idx, "Dimension");
		if (!lua_isnil(L, -1)) dimension = (int)lua_tointeger(L, -1);
		lua_pop(L, 1);
	} else if (!has_grid_index) {
		if (argCount >= 4 && !lua_isnil(L, 4)) {
			stage = (int)luaL_checkinteger(L, 4);
			stageSpecified = true;
		}
		if (argCount >= 5) type = (int)luaL_checkinteger(L, 5);
		if (argCount >= 6) shape = (int)luaL_optinteger(L, 6, 1);
		if (argCount >= 7) minVariant = (int)luaL_optinteger(L, 7, 0);
		if (argCount >= 8) maxVariant = (int)luaL_optinteger(L, 8, -1);
		if (argCount >= 9) minDifficulty = (int)luaL_optinteger(L, 9, 0);
		if (argCount >= 10) maxDifficulty = (int)luaL_optinteger(L, 10, 10);
		if (argCount >= 11) subtype = (int)luaL_optinteger(L, 11, -1);
		if (argCount >= 12) mode = (int)luaL_optinteger(L, 12, -1);
		if (argCount >= 13 && !lua_isnil(L, 13)) {
			allowed_doors = (int)luaL_optinteger(L, 13, -1);
			if (allowed_doors != -1) explicit_doors = true;
		}
		if (argCount >= 14 && !lua_isnil(L, 14)) {
			dimension = (int)luaL_checkinteger(L, 14);
		}
		if (argCount >= 15 && !lua_isnil(L, 15)) {
			force_doors = lua::luaL_optboolean(L, 15, false);
		}
	}

	if (!stageSpecified && type != 1) {
		stage = STB_SPECIAL_ROOMS;
	}

	int targetIdx = row * 13 + col;
	if (seed == 0) {
		seed = LuaCheckRoomPlacementSeed(L, 0, shape, col, row);
	}

	RoomConfig_Room* config = nullptr;
	if (IsValidStage(stage)) {
		uint32_t searchDoors = 0;
		if (explicit_doors && allowed_doors > 0) {
			searchDoors = (uint32_t)allowed_doors;
		} else if (!explicit_doors) {
			int autoDoors = CalculateRuntimeAutoDoors(targetIdx, shape, dimension);
			if (autoDoors > 0) {
				searchDoors = (uint32_t)autoDoors;
			}
		}
		config = g_Game->GetRoomConfig()->GetRandomRoom(
			seed, true, stage, type, shape,
			minVariant, maxVariant, minDifficulty,
			maxDifficulty, &searchDoors, subtype, mode
		);
		if (!config && force_doors && searchDoors != 0) {
			searchDoors = 0;
			config = g_Game->GetRoomConfig()->GetRandomRoom(
				seed, true, stage, type, shape,
				minVariant, maxVariant, minDifficulty,
				maxDifficulty, &searchDoors, subtype, mode
			);
		}
	}

	if (!config) {
		lua_pushnil(L);
		return 1;
	}

	if (!PrepareTargetLocation(level, col, row, config->Shape, dimension)) {
		lua_pushnil(L);
		return 1;
	}

	LevelGenerator_Room levelgenroom;
	levelgenroom.constructor(col, row, config->Shape);

	int newIdx = (int)g_Game->_nbRooms;
	if (newIdx < 507) {
		ClearRoomDescriptor(g_Game->_gridRooms[newIdx], newIdx);
	}

	bool placed = game->PlaceRoom(&levelgenroom, config, seed, 0);
	if (!placed) {
		lua_pushnil(L);
		return 1;
	}

	RoomDescriptor* desc = &g_Game->_gridRooms[g_Game->_nbRooms - 1];
	desc->ListIndex = (int)g_Game->_nbRooms - 1;
	ConnectRoomDoors(level, desc, targetIdx, config->Shape, allowed_doors, explicit_doors, dimension);

	lua::luabridge::UserdataPtr::push(L, desc, lua::GetMetatableKey(lua::Metatables::ROOM_DESCRIPTOR));
	return 1;
}

LUA_FUNCTION(Lua_LevelPlaceRoomAtDoor) {
	Level* level = lua::GetLuabridgeUserdata<Level*>(L, 1, lua::Metatables::LEVEL, "Level");
	int doorSlot = (int)luaL_checkinteger(L, 2);
	if (doorSlot < 0 || doorSlot >= 8) {
		return luaL_argerror(L, 2, "Invalid door slot (must be 0-7)");
	}

	RoomDescriptor* currentDesc = g_Game->GetCurrentRoomDesc();
	if (!currentDesc || !currentDesc->Data) {
		lua_pushnil(L);
		return 1;
	}

	int dimension = (currentDesc->Dimension >= 0) ? currentDesc->Dimension : (int)g_Game->_currentDimensionIdx;

	DoorSourceTarget doorSourceTarget = GetDoorSourceTarget(currentDesc->GridIndex, currentDesc->Data->Shape, doorSlot, false);
	if (!doorSourceTarget.IsValid()) {
		lua_pushnil(L);
		return 1;
	}

	const int targetCol = doorSourceTarget.target.x;
	const int targetRow = doorSourceTarget.target.y;
	const int targetIdx = doorSourceTarget.GetTargetIdx();

	RoomConfig_Room* config = nullptr;
	int allowed_doors = -1;
	bool explicit_doors = false;
	bool force_doors = false;
	uint32_t seed = 0;

	if (lua_istable(L, 3) || lua_gettop(L) < 3 || lua_isnil(L, 3)) {
		int stage = RoomConfig::GetStageID(level->_stage, level->_stageType, -1);
		bool stageSpecified = false;
		int type = 1;
		int shape = ROOMSHAPE_1x1;
		int minVariant = 0, maxVariant = -1, minDifficulty = 0, maxDifficulty = 10, subtype = -1, mode = -1;

		if (lua_istable(L, 3)) {
			lua_getfield(L, 3, "Stage");
			if (!lua_isnil(L, -1)) {
				stage = (int)lua_tointeger(L, -1);
				stageSpecified = true;
			}
			lua_pop(L, 1);

			lua_getfield(L, 3, "Type");
			if (!lua_isnil(L, -1)) type = (int)lua_tointeger(L, -1);
			lua_pop(L, 1);

			lua_getfield(L, 3, "Shape");
			if (!lua_isnil(L, -1)) shape = (int)lua_tointeger(L, -1);
			lua_pop(L, 1);

			lua_getfield(L, 3, "MinVariant");
			if (!lua_isnil(L, -1)) minVariant = (int)lua_tointeger(L, -1);
			lua_pop(L, 1);

			lua_getfield(L, 3, "MaxVariant");
			if (!lua_isnil(L, -1)) maxVariant = (int)lua_tointeger(L, -1);
			lua_pop(L, 1);

			lua_getfield(L, 3, "MinDifficulty");
			if (!lua_isnil(L, -1)) minDifficulty = (int)lua_tointeger(L, -1);
			lua_pop(L, 1);

			lua_getfield(L, 3, "MaxDifficulty");
			if (!lua_isnil(L, -1)) maxDifficulty = (int)lua_tointeger(L, -1);
			lua_pop(L, 1);

			lua_getfield(L, 3, "Subtype");
			if (!lua_isnil(L, -1)) subtype = (int)lua_tointeger(L, -1);
			lua_pop(L, 1);

			lua_getfield(L, 3, "Mode");
			if (!lua_isnil(L, -1)) mode = (int)lua_tointeger(L, -1);
			lua_pop(L, 1);

			lua_getfield(L, 3, "Doors");
			if (!lua_isnil(L, -1)) {
				allowed_doors = (int)lua_tointeger(L, -1);
				explicit_doors = true;
			}
			lua_pop(L, 1);

			lua_getfield(L, 3, "ForceDoors");
			if (!lua_isnil(L, -1)) {
				force_doors = lua_toboolean(L, -1);
			} else {
				lua_pop(L, 1);
				lua_getfield(L, 3, "FullMuzzle");
				if (!lua_isnil(L, -1)) {
					force_doors = lua_toboolean(L, -1);
				} else {
					lua_pop(L, 1);
					lua_getfield(L, 3, "Muzzle");
					if (!lua_isnil(L, -1)) {
						force_doors = lua_toboolean(L, -1);
					}
				}
			}
			lua_pop(L, 1);

			lua_getfield(L, 3, "Seed");
			if (!lua_isnil(L, -1)) seed = (uint32_t)lua_tointeger(L, -1);
			lua_pop(L, 1);

			lua_getfield(L, 3, "Dimension");
			if (!lua_isnil(L, -1)) dimension = (int)lua_tointeger(L, -1);
			lua_pop(L, 1);
		}

		if (!stageSpecified && type != 1) {
			stage = STB_SPECIAL_ROOMS;
		}

		if (seed == 0) seed = LuaCheckRoomPlacementSeed(L, 0, shape, targetCol, targetRow);
		int returnDoorSlot = -1;
		XY candidatePos = doorSourceTarget.target;
		for (int rSlot : GetPossibleReturnDoorSlots(doorSlot)) {
			DoorSourceTarget rst = GetDoorSourceTarget(doorSourceTarget.GetTargetIdx(), shape, rSlot, false);
			if (rst.IsValid()) {
				XY offset = doorSourceTarget.target - rst.source;
				XY candidate = doorSourceTarget.target + offset;
				if (candidate.x >= 0 && candidate.x < 13 && candidate.y >= 0 && candidate.y < 13) {
					DoorSourceTarget candRst = GetDoorSourceTarget(candidate.ToGridIdx(), shape, rSlot, false);
					if (candRst.ConnectsTo(doorSourceTarget)) {
						returnDoorSlot = rSlot;
						candidatePos = candidate;
						break;
					}
				}
			}
		}
		if (returnDoorSlot == -1) {
			returnDoorSlot = (doorSlot + 2) % 4;
		}

		if (IsValidStage(stage)) {
			uint32_t searchDoors = 0;
			if (explicit_doors && allowed_doors > 0) {
				searchDoors = (uint32_t)allowed_doors;
			} else if (!explicit_doors) {
				int autoDoors = CalculateRuntimeAutoDoors(candidatePos.ToGridIdx(), shape, dimension);
				if (returnDoorSlot >= 0) {
					autoDoors |= (1 << returnDoorSlot);
				}
				if (autoDoors > 0) {
					searchDoors = (uint32_t)autoDoors;
				}
			}
			config = g_Game->GetRoomConfig()->GetRandomRoom(seed, true, stage, type, shape, minVariant, maxVariant, minDifficulty, maxDifficulty, &searchDoors, subtype, mode);
			if (!config && force_doors && searchDoors != 0) {
				searchDoors = 0;
				config = g_Game->GetRoomConfig()->GetRandomRoom(seed, true, stage, type, shape, minVariant, maxVariant, minDifficulty, maxDifficulty, &searchDoors, subtype, mode);
			}
		}
		if (!config) {
			lua_pushnil(L);
			return 1;
		}
	} else if (lua_type(L, 3) == LUA_TUSERDATA) {
		config = lua::GetLuabridgeUserdata<RoomConfig_Room*>(L, 3, lua::Metatables::CONST_ROOM_CONFIG_ROOM, "RoomConfig");
		if (!config) {
			lua_pushnil(L);
			return 1;
		}
		if (lua_gettop(L) >= 4 && !lua_isnil(L, 4)) {
			allowed_doors = (int)luaL_checkinteger(L, 4);
			if (allowed_doors != -1) explicit_doors = true;
		}
		seed = (uint32_t)luaL_optinteger(L, 5, 0);
		if (lua_gettop(L) >= 6 && !lua_isnil(L, 6)) {
			dimension = (int)luaL_checkinteger(L, 6);
		}
	} else if (lua_isinteger(L, 3) && lua_isinteger(L, 4)) {
		int stb = (int)luaL_checkinteger(L, 3);
		int type = (int)luaL_checkinteger(L, 4);
		int variant = (int)luaL_optinteger(L, 5, 0);
		if (IsValidStage(stb)) {
			config = g_Game->GetRoomConfig()->GetRoomByStageTypeAndVariant(stb, type, variant, -1);
		}
		if (!config) {
			lua_pushnil(L);
			return 1;
		}
		if (lua_gettop(L) >= 6 && !lua_isnil(L, 6)) {
			allowed_doors = (int)luaL_checkinteger(L, 6);
			if (allowed_doors != -1) explicit_doors = true;
		}
		seed = (uint32_t)luaL_optinteger(L, 7, 0);
		if (lua_gettop(L) >= 8 && !lua_isnil(L, 8)) {
			dimension = (int)luaL_checkinteger(L, 8);
		}
	} else {
		lua_pushnil(L);
		return 1;
	}

	XY placementPos = doorSourceTarget.target;
	bool foundPlacement = (config->Shape == ROOMSHAPE_1x1);
	for (int rSlot : GetPossibleReturnDoorSlots(doorSlot)) {
		DoorSourceTarget rst = GetDoorSourceTarget(doorSourceTarget.GetTargetIdx(), config->Shape, rSlot, false);
		if (rst.IsValid()) {
			XY offset = doorSourceTarget.target - rst.source;
			XY candidate = doorSourceTarget.target + offset;
			if (candidate.x >= 0 && candidate.x < 13 && candidate.y >= 0 && candidate.y < 13) {
				DoorSourceTarget candRst = GetDoorSourceTarget(candidate.ToGridIdx(), config->Shape, rSlot, false);
				if (candRst.ConnectsTo(doorSourceTarget)) {
					placementPos = candidate;
					foundPlacement = true;
					break;
				}
			}
		}
	}

	if (!foundPlacement) {
		lua_pushnil(L);
		return 1;
	}

	const int finalCol = placementPos.x;
	const int finalRow = placementPos.y;
	const int finalIdx = placementPos.ToGridIdx();

	if (seed == 0) {
		seed = LuaCheckRoomPlacementSeed(L, 0, config->Shape, finalCol, finalRow);
	}

	if (!PrepareTargetLocation(level, finalCol, finalRow, config->Shape, dimension)) {
		lua_pushnil(L);
		return 1;
	}

	currentDesc = g_Game->GetCurrentRoomDesc();
	if (!currentDesc || !currentDesc->Data) {
		lua_pushnil(L);
		return 1;
	}

	LevelGenerator_Room levelgenroom;
	levelgenroom.constructor(finalCol, finalRow, config->Shape);

	Game* game = (Game*)level;
	int newIdx = (int)g_Game->_nbRooms;
	if (newIdx < 507) {
		ClearRoomDescriptor(g_Game->_gridRooms[newIdx], newIdx);
	}

	bool placed = game->PlaceRoom(&levelgenroom, config, seed, 0);
	if (!placed) {
		lua_pushnil(L);
		return 1;
	}

	RoomDescriptor* desc = &g_Game->_gridRooms[g_Game->_nbRooms - 1];
	desc->ListIndex = (int)g_Game->_nbRooms - 1;
	ConnectRoomDoors(level, desc, finalIdx, config->Shape, allowed_doors, explicit_doors, dimension);

	for (int returnDoorSlot : GetPossibleReturnDoorSlots(doorSlot)) {
		DoorSourceTarget returnSourceTarget = GetDoorSourceTarget(finalIdx, config->Shape, returnDoorSlot, false);
		if (returnSourceTarget.ConnectsTo(doorSourceTarget)) {
			desc->Doors[returnDoorSlot] = doorSourceTarget.GetSourceIdx();
			desc->AllowedDoors |= (1 << returnDoorSlot);

			currentDesc->Doors[doorSlot] = returnSourceTarget.GetSourceIdx();
			currentDesc->AllowedDoors |= (1 << doorSlot);
			break;
		}
	}

	Room* currentRoom = g_Game->GetCurrentRoom();
	if (currentRoom) {
		currentRoom->MakeDoor(doorSlot);
		GridEntity_Door* door = currentRoom->GetDoor(doorSlot);
		if (door) {
			door->_targetRoomIdx = doorSourceTarget.GetTargetIdx();
			door->SetRoomTypes(currentDesc->Data->Type, config->Type);
			if (currentRoom->GetFrameCount() > 0) door->play_animation();
			Entity_Effect* outline = currentRoom->_redDoorOutlines[doorSlot];
			if (outline) {
				outline->Remove();
				currentRoom->_redDoorOutlines[doorSlot] = nullptr;
			}
		}
	}

	lua::luabridge::UserdataPtr::push(L, desc, lua::GetMetatableKey(lua::Metatables::ROOM_DESCRIPTOR));
	return 1;
}

LUA_FUNCTION(Lua_LevelRemoveRoom) {
	Level* level = lua::GetLuabridgeUserdata<Level*>(L, 1, lua::Metatables::LEVEL, "Level");
	int gridIdx = 0;
	int dimension = -1;
	if (lua_gettop(L) >= 3 && lua_isinteger(L, 2) && lua_isinteger(L, 3)) {
		int col = (int)luaL_checkinteger(L, 2);
		int row = (int)luaL_checkinteger(L, 3);
		gridIdx = row * 13 + col;
		if (lua_gettop(L) >= 4 && !lua_isnil(L, 4)) {
			dimension = (int)luaL_checkinteger(L, 4);
		}
	} else {
		gridIdx = (int)luaL_checkinteger(L, 2);
		if (lua_gettop(L) >= 3 && !lua_isnil(L, 3)) {
			dimension = (int)luaL_checkinteger(L, 3);
		}
	}

	if (gridIdx < 0 || gridIdx > 168) {
		lua_pushboolean(L, false);
		return 1;
	}

	RoomDescriptor* currentDesc = g_Game->GetCurrentRoomDesc();
	if (dimension < 0) {
		dimension = (currentDesc && currentDesc->Dimension >= 0) ? currentDesc->Dimension : (int)g_Game->_currentDimensionIdx;
	}

	RoomDescriptor* desc = g_Game->GetRoomByIdx(gridIdx, dimension);
	if (!desc || !desc->Data) {
		lua_pushboolean(L, false);
		return 1;
	}

	bool removed = InternalRemoveRoom(level, desc);
	lua_pushboolean(L, removed);
	return 1;
}

LUA_FUNCTION(Lua_LevelGetLastBossRoomListIndex) {
	lua_pushinteger(L, (lua_Integer)g_Game->_lastBossRoomListIdx);
	return 1;
}

LUA_FUNCTION(Lua_LevelSetLastBossRoomListIndex) {
	int idx = (int)luaL_checkinteger(L, 2);
	g_Game->_lastBossRoomListIdx = idx;
	return 0;
}

HOOK_METHOD(LuaEngine, RegisterClasses, () -> void) {
	super();

	lua::LuaStackProtector protector(_state);

	luaL_Reg functions[] = {
		{ "CanSpawnDoorOutline", Lua_LevelCanSpawnDoorOutline },
		{ "HasAbandonedMineshaft", Lua_LevelHasAbandonedMineshaft },
		{ "HasMirrorDimension", Lua_LevelHasMirrorDimension },
		{ "HasPhotoDoor", Lua_LevelHasPhotoDoor },
		{ "SetName", lua_LevelSetName },
		{ "IsStageAvailable", lua_LevelIsStageAvailable },
		{ "GetDimension", Lua_GetDimension},
		{ "GetForceSpecialQuest", Lua_GetForceSpecialQuest },
		{ "SetForceSpecialQuest", Lua_SetForceSpecialQuest },
		{ "GetMyosotisPickups", Lua_GetMyosotisPickups },

		{ "SetGreedWavesClearedWithoutRedHeartDamage", lua_LevelSetGreedWavesClearedWithoutRedHeartDamage },
		{ "GetGreedWavesClearedWithoutRedHeartDamage", lua_LevelGetGreedWavesClearedWithoutRedHeartDamage },

		{ "CanPlaceRoom", Lua_LevelCanPlaceRoom },
		{ "TryPlaceRoom", Lua_LevelTryPlaceRoom },
		{ "CanPlaceRoomAtDoor", Lua_LevelCanPlaceRoomAtDoor },
		{ "TryPlaceRoomAtDoor", Lua_LevelTryPlaceRoomAtDoor },
		{ "FindValidRoomPlacementLocations", Lua_LevelFindValidRoomPlacementLocations },
		{ "GetNeighboringRooms", Lua_LevelGetNeighboringRooms },
		{ "GetGenerationRNG", Lua_LevelGetGenerationRNG },
		{ "UpdateDoors", Lua_LevelUpdateDoors },
		{ "PlaceRandomRoom", Lua_LevelPlaceRandomRoom },
		{ "PlaceRoomAtDoor", Lua_LevelPlaceRoomAtDoor },
		{ "RemoveRoom", Lua_LevelRemoveRoom },
		{ "GetLastBossRoomListIndex", Lua_LevelGetLastBossRoomListIndex },
		{ "SetLastBossRoomListIndex", Lua_LevelSetLastBossRoomListIndex },

		{ NULL, NULL }
	};

	lua::RegisterFunctions(_state, lua::Metatables::LEVEL, functions);
}