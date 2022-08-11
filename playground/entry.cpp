#include <imgui.h>
#include <imgui_stdlib.h>
#include "parms/hello.h"
#include "parmscript.h"

#include <iostream>

static std::string const testscript = R"(
label 'A Little Test:'
toggle 'x' {label='Enable Group Foo', default=true}
text 'name' {label='Name', default='Foo\nBar\n!@#$%^&*()_+""', multiline=true}
group 'foo' {label='Foo', disablewhen='{x}==false'}
  label 'A Int Value:' {joinnext=true}
  int 'a' {max=1024, min=1}
  float 'b' {default=1024, ui='drag', speed=1}
  color 'color1' {hdr=true, default={1,0,0}}
  color 'yellow' {hdr=true, default={1,1,0}}
  color 'green' {hdr=true, default={0,1,0,1}, alpha=false, wheel=true}
  color 'color2' {alpha=false, hsv=true, wheel=true, default={0.3,0.1,0.4,1.0}}
  button 'sayhi' {label='Say Hi'}
  int 'c'
  struct 'X'
    float 'a'
    float 'b'
    double 'c'
  endstruct 'X'
endgroup 'foo'
)";

namespace ImGui {

Hello hello;
std::unordered_set<std::string> modified;

void ShowDemoWindow(bool* opened)
{
  hello.sayhi=[](){
    ParmSet ps;
    hello.x = ps.loadScript(testscript);
  };
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
