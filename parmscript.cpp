#include "parmscript.h"
#include "deps/imgui/imgui.h"
#include "imgui_internal.h"
#include "sol/forward.hpp"
#include "sol/stack_core.hpp"
#include "sol/traits.hpp"

#include <array>
#include <imgui.h>
#include <imgui_stdlib.h>

#include <memory>
#include <objidlbase.h>
#include <sol/sol.hpp>
#include <stdint.h>
#include <winuser.h>
extern "C" {
#include <lua.h>
}

static const char parmexpr_src[] = {
#include <parmexpr.lua.h>
};

#ifdef DEBUG
#define WARN(...) fprintf(stderr, __VA_ARGS__)
#define INFO(...) fprintf(stdout, __VA_ARGS__)
#else
#define WARN(...) /*nothing*/
#define INFO(...) /*nothing*/
#endif

bool Parm::updateInspector(Parm::hashset<Parm::string>& modified)
{
  bool imdirty = false;
  bool displayChildren = true;

  string disablewhen = getmeta<string>("disablewhen", "");
  // TODO: expand {/path/to/parm} to its value
  // TODO: expand {menu:/path/to/parm:item} to its value
  // TODO: translate != into ~=, || into or, && into and, ! into not
  // TODO: evaluate disablewhen expr in Lua
  if (!disablewhen.empty()) {
    ImGui::BeginDisabled(false);
  }

  if (ui_type_ == ui_type_enum::GROUP) {
    if (!ImGui::CollapsingHeader(label_.c_str(), ImGuiTreeNodeFlags_DefaultOpen)) {
      displayChildren = false;
    }
  } else if (ui_type_ == ui_type_enum::STRUCT) {
    if (!ImGui::TreeNodeEx(label_.c_str(), ImGuiTreeNodeFlags_Framed | ImGuiTreeNodeFlags_DefaultOpen)) {
      displayChildren = false;
    }
  } else if (ui_type_ == ui_type_enum::LIST) {
    displayChildren = false;
    int numitems = listValues_.size();
    if (ImGui::InputInt(label_.c_str(), &numitems)) {
      resizeList(numitems);
    }
    for (int i=0; i<numitems; ++i) {
      // TODO: this should be indexed
      for (auto field: listValues_[i]->fields_) {
        imdirty |= field->updateInspector(modified);
      }
      if (i+1<numitems) {
        ImGui::Separator();
      }
    }
  } else if (ui_type_ == ui_type_enum::LABEL) {
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
      int min = getmeta<int>("min", 0);
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
      float min = getmeta<float>("min", 0.f);
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
    case value_type_enum::FLOAT2:
    {
      float2 v = std::get<float2>(value_);
      string ui = getmeta<string>("ui", "drag");
      float min = getmeta<float>("min", 0.f);
      float max = getmeta<float>("max", 1.f);
      float speed = getmeta<float>("speed", 1.f);
      if (ui == "drag") {
        imdirty = ImGui::DragFloat2(label_.c_str(), &v.x, speed);
      } else if (ui == "slider") {
        imdirty = ImGui::SliderFloat2(label_.c_str(), &v.x, min, max);
      } else {
        imdirty = ImGui::InputFloat2(label_.c_str(), &v.x);
      }
      if (imdirty)
        value_ = v;
      break;
    }
    case value_type_enum::FLOAT3:
    {
      float3 v = std::get<float3>(value_);
      string ui = getmeta<string>("ui", "drag");
      float min = getmeta<float>("min", 0.f);
      float max = getmeta<float>("max", 1.f);
      float speed = getmeta<float>("speed", 1.f);
      if (ui == "drag") {
        imdirty = ImGui::DragFloat3(label_.c_str(), &v.x, speed);
      } else if (ui == "slider") {
        imdirty = ImGui::SliderFloat3(label_.c_str(), &v.x, min, max);
      } else {
        imdirty = ImGui::InputFloat3(label_.c_str(), &v.x);
      }
      if (imdirty)
        value_ = v;
      break;
    }
    case value_type_enum::FLOAT4:
    {
      float4 v = std::get<float4>(value_);
      string ui = getmeta<string>("ui", "drag");
      float min = getmeta<float>("min", 0.f);
      float max = getmeta<float>("max", 1.f);
      float speed = getmeta<float>("speed", 1.f);
      if (ui == "drag") {
        imdirty = ImGui::DragFloat3(label_.c_str(), &v.x, speed);
      } else if (ui == "slider") {
        imdirty = ImGui::SliderFloat3(label_.c_str(), &v.x, min, max);
      } else {
        imdirty = ImGui::InputFloat3(label_.c_str(), &v.x);
      }
      if (imdirty)
        value_ = v;
      break;
    }
    case value_type_enum::DOUBLE:
    {
      double v = std::get<double>(value_);
      float speed = getmeta<double>("speed", 1.0);
      imdirty = ImGui::InputDouble(label_.c_str(), &v, speed);
      if (imdirty)
        value_ = v;
      break;
    }
    case value_type_enum::STRING:
    {
      string *v = std::get_if<string>(&value_);
      if (!v)
        break;
      bool multiline = getmeta<bool>("multiline", false);
      if (multiline)
        imdirty = ImGui::InputTextMultiline(label_.c_str(), v);
      else
        imdirty = ImGui::InputText(label_.c_str(), v);
      break;
    }
    case value_type_enum::COLOR:
    {
      color v = std::get<color>(value_);
      bool alpha = getmeta<bool>("alpha", false);
      bool hsv = getmeta<bool>("hsv", false);
      bool hdr = getmeta<bool>("hdr", false);
      bool wheel = getmeta<bool>("wheel", false);
      bool picker = getmeta<bool>("picker", false);

      uint32_t flags = 0;
      if (alpha)
        flags |= ImGuiColorEditFlags_AlphaBar | ImGuiColorEditFlags_AlphaPreview | ImGuiColorEditFlags_AlphaPreviewHalf;
      else
        flags |= ImGuiColorEditFlags_NoAlpha;

      if (hsv)
        flags |= ImGuiColorEditFlags_DisplayHSV;
      else
        flags |= ImGuiColorEditFlags_DisplayRGB;

      if (hdr)
        flags |= ImGuiColorEditFlags_Float;
      else
        flags |= ImGuiColorEditFlags_Uint8;

      if (wheel)
        flags |= ImGuiColorEditFlags_PickerHueWheel;

      if (alpha) {
        if (picker)
          imdirty = ImGui::ColorPicker4(label_.c_str(), &v.r, flags);
        else
          imdirty = ImGui::ColorEdit4(label_.c_str(), &v.r, flags);
      } else {
        if (picker)
          imdirty = ImGui::ColorPicker3(label_.c_str(), &v.r, flags);
        else
          imdirty = ImGui::ColorEdit3(label_.c_str(), &v.r, flags);
      }
      if (imdirty)
        value_ = v;
      break;
    }
    default:
      break;
    }
  } else if (ui_type_ == ui_type_enum::MENU) {
    std::vector<char const*> labels;
    labels.reserve(menu_labels_.size());
    for (auto const& label: menu_labels_) {
      labels.push_back(label.c_str());
    }
    if (ImGui::Combo(label_.c_str(), std::get_if<int>(&value_), labels.data(), labels.size()))
      imdirty = true;
  }

  if (displayChildren && !fields_.empty()) {
    for (auto child: fields_)
      imdirty |= child->updateInspector(modified);
  }
  if (ui_type_==ui_type_enum::STRUCT && displayChildren) {
    ImGui::TreePop();
  }

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
  auto self = sol::stack::get<ParmSet*>(lua, 1);
  auto parentid = sol::stack::get<int>(lua, 2);
  auto field = sol::stack::get<sol::table>(L, 3);
  if (parentid<0 || !field) {
    WARN("bad argument passed to processLuaParm()\n");
    return 0;
  }
  string ui   = field["ui"];
  string path = field["path"];
  string name = field["name"];
  string type = field["type"];
  auto   meta = field["meta"];
  string label = meta["label"].get_or(Parm::titleize(name));
  auto  defaultfield = meta["default"];
  Parm::value_type defaultval;
  INFO("processing field label(%s) ui(%s) type(%s) name(%s) ... ", label.c_str(), ui.c_str(), type.c_str(), name.c_str());

  auto parent = self->parms_[parentid];
  auto newparm = std::make_shared<Parm>(Parm(self));

  Parm::ui_type_enum uitype = Parm::ui_type_enum::FIELD;
  if (ui=="label")          uitype=Parm::ui_type_enum::LABEL;
  else if (ui=="separator") uitype=Parm::ui_type_enum::SEPARATOR;
  else if (ui=="spacer")    uitype=Parm::ui_type_enum::SPACER;
  else if (ui=="button")    uitype=Parm::ui_type_enum::BUTTON;
  else if (ui=="menu")      uitype=Parm::ui_type_enum::MENU;
  else if (ui=="group")     uitype=Parm::ui_type_enum::GROUP;
  else if (ui=="struct")    uitype=Parm::ui_type_enum::STRUCT;
  else if (ui=="list")      uitype=Parm::ui_type_enum::LIST;

  auto parseminmax = [&meta, &newparm](auto t) {
    using T = decltype(t);
    if (meta["min"].valid())
      newparm->setMeta<T>("min", meta["min"].get<T>());
    if (meta["max"].valid())
      newparm->setMeta<T>("max", meta["max"].get<T>());
    if (meta["speed"].valid())
      newparm->setMeta<T>("speed", meta["speed"].get<T>());
    if (!meta["ui"].valid() && meta["min"].valid() && meta["max"].valid())
      newparm->setMeta<string>("ui", "slider");
  };
  auto boolmeta = [&meta, &newparm](string const& key) {
    if (meta[key].valid())
      newparm->setMeta<bool>(key, meta[key].get<bool>());
  };
  INFO("1 ");

  Parm::value_type_enum valuetype = Parm::value_type_enum::NONE;
  if (type=="bool") {
    valuetype = Parm::value_type_enum::BOOL;
    defaultval = defaultfield.get_or(false);
  } else if (type=="int") {
    valuetype = Parm::value_type_enum::INT;
    defaultval = defaultfield.get_or(0);
    parseminmax(0);
  } else if (type=="float") {
    valuetype = Parm::value_type_enum::FLOAT;
    defaultval = defaultfield.get_or(0.f);
    parseminmax(0.f);
  } else if (type=="float2") {
    valuetype = Parm::value_type_enum::FLOAT2;
    auto vals = defaultfield.get_or(std::array<float, 2>{0,0});
    defaultval = Parm::float2{vals[0], vals[1]};
    parseminmax(0.f);
  } else if (type=="float3") {
    valuetype = Parm::value_type_enum::FLOAT3;
    auto vals = defaultfield.get_or(std::array<float, 3>{0,0,0});
    defaultval = Parm::float3{vals[0], vals[1], vals[2]};
    parseminmax(0.f);
  } else if (type=="float4") {
    valuetype = Parm::value_type_enum::FLOAT4;
    auto vals = defaultfield.get_or(std::array<float, 4>{0,0,0});
    defaultval = Parm::float4{vals[0], vals[1], vals[2], vals[3]};
    parseminmax(0.f);
  } else if (type=="color") {
    valuetype = Parm::value_type_enum::COLOR;
    auto vals = defaultfield.get_or(std::array<float, 4>{1,1,1,1});
    defaultval = Parm::color{vals[0], vals[1], vals[2], vals[3]};
    boolmeta("alpha");
    boolmeta("hsv");
    boolmeta("hdr");
    boolmeta("wheel");
    boolmeta("picker");
  } else if (type=="string") {
    valuetype = Parm::value_type_enum::STRING;
    defaultval.emplace<string>(defaultfield.get_or(string("")));
    boolmeta("multiline");
  }
  if (meta["ui"].valid())
    newparm->setMeta<string>("ui", meta["ui"].get<string>());
  if (meta["disablewhen"].valid())
    newparm->setMeta<string>("disablewhen", meta["disablewhen"].get<string>());
  boolmeta("joinnext");
  INFO("2 ");
  newparm->setup(name, path, label, uitype, valuetype, defaultval);

  if (uitype == Parm::ui_type_enum::MENU) {
    auto items  = meta["items"].get_or(std::vector<string>());
    auto labels = meta["itemlabels"].get_or(std::vector<string>());
    auto values = meta["itemvalues"].get_or(std::vector<int>());
    string nativedefault = "";
    if (!items.empty())
      nativedefault = items.front();
    std::string strdefault = defaultfield.get_or(nativedefault);
    auto itrdefault = std::find(items.begin(), items.end(), strdefault);
    int idxdefault = 0;
    if (itrdefault != items.end())
      idxdefault = itrdefault-items.begin();
    newparm->setMenu(items, idxdefault, labels, values);
  }
  INFO("3 ");
  parent->addField(newparm);
  self->parms_.push_back(newparm);
  int  newid = self->parms_.size()-1;
  sol::stack::push(lua, newid);
  INFO("done.\n");
  return 1;
}

bool ParmSet::loadScript(std::string const& s)
{
  loaded_ = false;
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
  root_  = std::make_shared<Parm>(nullptr);
  parms_ = {root_};
  parmlut_.clear();
  parmlut_[""] = root_;

  sol::state_view L{lua_};
  try {
    L.set_function("process", processLuaParm);
    L.set("cpp", this);
    L.safe_script(R"==(
local function dofield(cpp, parentid, field)
  local id = process(cpp, parentid, field)
  if field.fields and #field.fields > 0 then
    for _, v in pairs(field.fields) do
      dofield(cpp, id, v)
    end
  end
end

for _,v in pairs(parmscript.root.fields) do
  dofield(cpp, 0, v)
end
    )==");
  } catch(std::exception const& e) {
    WARN("exception: %s\n", e.what());
  }
  loaded_ = true;
  return true;
}

bool ParmSet::updateInspector()
{
  dirty_.clear();
  root_->updateInspector(dirty_);
  return !dirty_.empty();
}