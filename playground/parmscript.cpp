#include "parmscript.h"

#include <imgui.h>
#include <imgui_stdlib.h>

extern "C" {
#include <lua.h>
}

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