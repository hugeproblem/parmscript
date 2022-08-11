#include "parmscript.h"

#include <imgui.h>
#include <imgui_stdlib.h>

#include <sol/sol.hpp>
extern "C" {
#include <lua.h>
}

static const char parmexpr_src[] = {
#include <parmexpr.lua.h>
};

#ifdef DEBUG
#define WARN(...) fprintf(stderr, __VA_ARGS__)
#else
#define WARN(...) /*nothing*/
#endif

bool Parm::updateInspector(Parm::hashset<Parm::string>& modified)
{
  string disablewhen = "";
  bool imdirty = false;
  if (auto di=meta_.find("disablewhen"); di!=meta_.end()) {
    if (auto *sptr=std::get_if<string>(&di->second)) {
      disablewhen = *sptr;
    }
  }
  // TODO: expand {/path/to/parm} to its value
  // TODO: expand {menu:/path/to/parm:item} to its value
  // TODO: translate != into ~=, || into or, && into and, ! into not
  // TODO: evaluate disablewhen expr in Lua
  if (!disablewhen.empty()) {
    ImGui::BeginDisabled(false);
  }

  if (ui_type_ == ui_type_enum::LABEL) {
    ImGui::TextUnformatted(label_.c_str());
  } else if (ui_type_ == ui_type_enum::BUTTON) {
    if (ImGui::Button(label_.c_str())) {
      modified.insert(path_);
      // TODO: callback
    }
  } else if (ui_type_ == ui_type_enum::FIELD) {
    switch(expected_value_type_) {
    case value_type_enum::INT:
    {
      int  val=0;
      int* iptr = std::get_if<int>(&value_);
      if (iptr)
        val = *iptr;
      if (ImGui::DragInt(label_.c_str(), &val)) {
        value_ = val;
        modified.insert(path_);
        imdirty = true;
      }
      break;
    }
    // TODO: more
    }
  } // TODO: more

  if (!disablewhen.empty()) {
    ImGui::EndDisabled();
  }
  return imdirty;
} 

bool ParmSet::loadScript(std::string const& s)
{
  if (lua_) {
    lua_close(lua_);
  }
  lua_ = luaL_newstate();
  if (!lua_) {
    return false;
  }
  luaL_openlibs(lua_);
  if (LUA_OK != luaL_loadbufferx(lua_, parmexpr_src, sizeof(parmexpr_src)-1, "parmexpr", "t")) {
    WARN("failed to load parmexpr\n");
    return false;
  }
  if (LUA_OK == lua_pcall(lua_, 0, 1, 0)) {
    lua_pushvalue(lua_, -1); // backup for later call
    lua_setglobal(lua_, "parmexpr");
  } else {
    WARN("failed to call parmexpr\n");
    return false;
  }
  lua_pushlstring(lua_, s.c_str(), s.size());
  if (LUA_OK == lua_pcall(lua_, 1, 1, 0)) {
    lua_setglobal(lua_, "parmscript");
  } else {
    WARN("failed to parse parmscript: error: %s\n", luaL_optstring(lua_, -1, "unknown"));
    return false;
  }

  sol::state_view L{lua_};
  try {
    L.safe_script(R"(for i,v in pairs(parmscript.allParms()) do print(i,v) end)");
  } catch(std::exception const& e) {
    WARN("exception: %s\n", e.what());
  }
  return true;
}
