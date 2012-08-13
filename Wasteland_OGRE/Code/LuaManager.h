#include "StdAfx.h"
#include <lua.hpp>

//#include <OgreSingleton.h>

#ifndef _LUA_MANAGER_H_
#define _LUA_MANAGER_H_

#include "LevelData.h"

//Make my life easier
typedef int (*luaFunction)(lua_State*);

/*
lua_State* luaTest;
luaTest = lua_open();

luaL_openlibs(luaTest);
luaL_dofile(luaTest,"resource\\lua\\test.lua");
int sum;
{
	lua_getglobal(luaTest,"add");
	lua_pushnumber(luaTest,5);
	lua_pushnumber(luaTest,2);
	lua_call(luaTest,2,1);
	sum = (int)lua_tointeger(luaTest,-1);
	lua_pop(luaTest,1);
}
lua_close(luaTest);
*/

class LuaManager : public Ogre::Singleton<LuaManager>
{
public:
	LuaManager();
	~LuaManager();

	lua_State* getLuaState();

	void Setup(std::string luaListFileName);

	void registerFunction(std::string funcName,luaFunction funcPtr);

	//Calls the function, but calling code must handle parameters and return values
	//no return values
	void callFunction(const std::string& funcName);
	//return values
	void callFunction(const std::string& funcName,int expectedNumReturn);

	//If prepFunction is called, then callFunction must be called afterwards.
	//All prepFunction does is allow the calling code to push parameters onto the stack for the lua function
	void prepFunction(const std::string& funcName);
	void callFunction(int paramNum,int retNum);

	void pushFunctionArg(boost::variant<int,double,std::string> arg);
	void pushFunctionArgVector(const Ogre::Vector3& vector);
	void pushFunctionArgVector(const btVector3& vector);

	void addEntity(const std::string& name,LevelData::BaseEntity* entity);
	LevelData::BaseEntity* getEntity(const std::string& name) { return _entities[name]; }
	void removeEntity(const std::string& name) { _entities.erase(_entities.find(name)); }
	void purgeEntities();

	void activateEntity(const std::string& name, bool value);

private:
	lua_State* luaState;

	std::map<std::string,LevelData::BaseEntity*> _entities;

	LuaManager(const LuaManager&);
	LuaManager& operator=(const LuaManager&);
};

//Helper class for LuaManager
class argVisitor : public boost::static_visitor<>
{
public:
	argVisitor() : numeric(0),rational(0.0),string("") {}
	int numeric;
	double rational;
	std::string string;

	void clear() { numeric = 0; rational = 0.0; string = ""; }

	void getType(bool* num,bool* rat,bool* str);

	void operator()(const int& i);
	void operator()(const std::string& str);
	void operator()(const double& d);
private:
	//bool _isN,_isR,_isD;
};

//==========================================
//Functions that will be registered with Lua
//==========================================

//Activation function. Allows interface between LuaManager and Lua without luabind or tolua++ or whatever.
int activate(lua_State* lua);

//Allows Lua scripts to change in-game entity names.
int changeEntityName(lua_State* lua);

//Allows Lua to print through std::cout directly.
int printDebug(lua_State* lua);

//Fast distance check function available through Lua.
int distanceCheck(lua_State* lua);

#endif