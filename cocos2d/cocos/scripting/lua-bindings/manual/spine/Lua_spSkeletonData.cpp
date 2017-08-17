﻿/*
	add by wss 16/6/24
	为cocos的spine增加接口
	*/

#include "Lua_spSkeletonData.hpp"
#include "tolua_fix.h"
#include "LuaBasicConversions.h"
#include "../../../../editor-support/spine/spine-cocos2dx.h"
#include "../../../../editor-support/spine/Cocos2dAttachmentLoader.h"
#include "../../../../editor-support/spine/extension.h"

using namespace spine;

//static int
static int lua_cocos2dx_SkeletonData_create(lua_State* L)
{
	if (nullptr == L){
		return 0;
	}
	int argc = 0;

#if COCOS2D_DEBUG >= 1
	tolua_Error tolua_err;
	if (!tolua_isusertable(L, 1, "sp.SkeletonData", 0, &tolua_err)) goto tolua_lerror;
#endif 
		argc = lua_gettop(L) - 1;
	if (2 == argc)
	{
#if COCOS2D_DEBUG >= 1 
		if (!tolua_isstring(L, 2, 0, &tolua_err) || !tolua_isstring(L, 3, 0, &tolua_err))
		{
			goto tolua_lerror;
		}
#endif
		const char* skeletonDataFile = tolua_tostring(L, 2, "");
		const char* atlasFile = tolua_tostring(L, 3, "");

		/*spAtlas* atlas = spAtlas_createFromFile(atlasFile, NULL);
		CCAssert(atlas, "Error reading atlas file.");
		spSkeletonJson* json = spSkeletonJson_create(atlas);
		spSkeletonData* data = spSkeletonJson_readSkeletonDataFile(json, skeletonDataFile);
		CCAssert(data, json->error ? json->error : "Error reading skeleton data.");
		spSkeletonJson_dispose(json);*/

		spAtlas* atlas = spAtlas_createFromFile(atlasFile, 0);
		spAttachmentLoader* atlasLod = SUPER(Cocos2dAttachmentLoader_create(atlas));

		spSkeletonJson* json = spSkeletonJson_createWithLoader(atlasLod);
		spSkeletonData* data = spSkeletonJson_readSkeletonDataFile(json, skeletonDataFile);
		CCASSERT(data, json->error ? json->error : "Error reading skeleton data.");
		spSkeletonJson_dispose(json);

		tolua_pushusertype(L, (void*)data, "sp.SkeletonData");
		tolua_register_gc(L, lua_gettop(L));
		return 1;

	}
	luaL_error(L, "'create' function of SkeletonData has wrong number of arguments: %d, was expecting %d\n", argc, 2);

#if COCOS2D_DEBUG >= 1
tolua_lerror:
	tolua_error(L, "#ferror in function 'lua_cocos2dx_SkeletonData_create'.", &tolua_err);
#endif
	return 0;
}

int lua_cocos2dx_SkeletonData_finalize(lua_State* L)
{
	spSkeletonData* data = (spSkeletonData*)tolua_tousertype(L, 1, 0);
	CC_SAFE_DELETE(data);
	CCLOG("sp.SkeletonData freed");
	return 0;
}

//static void
static void tolua_register_spSkeletonData(lua_State* tolua_S)
{
	tolua_usertype(tolua_S, "sp.SkeletonData");
	tolua_cclass(tolua_S, "SkeletonData", "sp.SkeletonData", "", lua_cocos2dx_SkeletonData_finalize);

	tolua_beginmodule(tolua_S, "SkeletonData");
	tolua_function(tolua_S, "create", lua_cocos2dx_SkeletonData_create);
	tolua_endmodule(tolua_S);

	std::string typeName = typeid(spSkeletonData).name();
	g_luaType[typeName] = "sp.SkeletonData";
	g_typeCast["SkeletonData"] = "sp.SkeletonData";
}

int register_spSkeletonData_manual(lua_State* L)
{
	if (nullptr == L)
		return 0;

	tolua_open(L);
	tolua_module(L, "sp", 0);
	tolua_beginmodule(L, "sp");
	tolua_register_spSkeletonData(L);
	tolua_endmodule(L);

	return 0;
}


