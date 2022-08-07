#include <imgui.h>
#include <imgui_stdlib.h>
#include "hello.h"
#include "hello_imgui_inspector.inl"

namespace ImGui {

Hello hello;
std::unordered_set<std::string> modified;


void ShowDemoWindow(bool* opened)
{
  if (ImGui::Begin("My Demo", opened)) {
    if (ImGuiInspect(hello, modified)) {
      ImGui::Text("Modified Items:");
      for (auto const& item: modified)
        ImGui::Text("%s", item.c_str());
    }
  }
  ImGui::End();
}

}
