#include "parmscript.h"
#include "deps/imgui/imgui.h"

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
  bool imdirty = false;

  string disablewhen = getmeta<string>("disablewhen", "");
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
    case value_type_enum::BOOL:
    {
      bool v = std::get<bool>(value_);
      if (ImGui::Checkbox(label_.c_str(), &v)) {
        value_ = v;
        imdirty = true;
      }
      break;
    }
    case value_type_enum::INT:
    {
      int v = std::get<int>(value_);
      string ui = getmeta<string>("ui", "drag");
      int min = getmeta<int>("min", 0)
      int max = getmeta<int>("max", 100);
      int speed = getmeta<int>("speed", 1);
      if (ui == "drag") {
        imdirty = ImGui::DragInt(label_.c_str(), &v, speed);
      } else if (ui == "slider") {
        imdirty = ImGui::SliderInt(label_.c_str(), &v, min, max);
      } else {
        imdirty = ImGui::InputInt(label_.c_str(), &v);
      }
      if (imdirty)
        value_ = v;
      break;
    }
    case value_type_enum::FLOAT:
    {
      float v = std::get<float>(value_);
      string ui = getmeta<string>("ui", "drag");
      float min = getmeta<float>("min", 0.f)
      float max = getmeta<float>("max", 1.f);
      float speed = getmeta<float>("speed", 1.f);
      if (ui == "drag") {
        imdirty = ImGui::DragFloat(label_.c_str(), &v, speed);
      } else if (ui == "slider") {
        imdirty = ImGui::SliderFloat(label_.c_str(), &v, min, max);
      } else {
        imdirty = ImGui::InputFloat(label_.c_str(), &v);
      }
      if (imdirty)
        value_ = v;
      break;
    }
    // TODO: more
    }
  } // TODO: more

  if (!disablewhen.empty()) {
    ImGui::EndDisabled();
  }
  if (imdirty)
    modified.insert(path_);
  if (getmeta<bool>("joinnext", false))
    ImGui::SameLine();
  return imdirty;
} 

int ParmSet::processLuaParm(lua_State* lua)
{
  sol::state_view L{lua};

  return 0;
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
    L.set_function("process", processLuaParm);
    L.safe_script("process(parmscript.root())");
  } catch(std::exception const& e) {
    WARN("exception: %s\n", e.what());
  }
  return true;
}
