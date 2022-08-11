#include <imgui.h>
#include <imgui_stdlib.h>
#include "parms/hello.h"
#include "parmscript.h"

#include <iostream>

namespace ImGui {

Hello hello;
std::unordered_set<std::string> modified;


void ShowDemoWindow(bool* opened)
{
  hello.sayhi=[](){std::cout<<"Hello!"<<std::endl;};
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
