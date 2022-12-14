#include <imgui.h>
#include <imgui_stdlib.h>
#include "parms/hello.h"
#include "parmscript.h"
extern "C" {
// Only for Lua API test
#include <lua.h>
#include <lualib.h>
#include <lauxlib.h>
}
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
  color 'yellow' {hdr=true, alpha=true, default={1,1,0,0.5}}
  color 'green' {hdr=true, default={0,1,0,1}, alpha=false, wheel=true}
  color 'color2' {alpha=true, hsv=true, wheel=true, default={0.3,0.1,0.4,1.0}}
  button 'sayhi' {label='Say Hi'}
  int 'c'
  struct 'X'
    float 'a'
    float 'b'
    double 'c'
  endstruct 'X'
endgroup 'foo'
spacer ''
spacer ''
separator ''
toggle 'y'
group 'bar' {closed=true, label='BarBarBar'}
  float2 'pos' {disablewhen='{length:Points}==0'}
  menu 'mode' {
    class='Mode', label='Mode',
    items={'a','b','c'}, default='b',
    itemlabels={'Apple','Banana','Coffe'},
    itemvalues={4,8,16} }
  color 'color3' {default={0.8,0.2,0.2,1.0}, disablewhen='{mode}!={menu:mode::a}'}
endgroup 'bar'
list 'Points' {class='Point'}
  float3 "pos" {label="Position"}
  float3 "N" {label="Normal", default={0,1,0}, min=-1, max=1}
endlist 'Points'
)";

static std::string luaapitest_src = R"(
local ParmSet = require('ParmSet')
parms = ParmSet.new()
parms:loadScript([[
  button 'buttttton'
  int 'iiiii' { default=2 }
  text 'sss' { default = 'hi there' }
  struct 'Foo'
    float 'x' { default=3 }
  endstruct 'Foo'
]])
parms:updateInspector()
assert(parms['iiiii']==2, string.format('iiiii==%q', parms['iiiii']))
for i,v in pairs(parms['']) do print(i,v) end
)";

namespace ImGui {

Hello hello;
std::unordered_set<std::string> modified;

static ParmSet& dynaParms()
{
  static ParmSet ps;
  return ps;
}

void ShowDemoWindow(bool* opened)
{
  hello.sayhi=[](){
    printf("hello!\n");
    hello.name = testscript;
    hello.x = dynaParms().loadScript(testscript);
  };
  hello.test_lua=[](){
    lua_State* L = luaL_newstate();
    luaL_openlibs(L);
    ParmSet::exposeToLua(L);
    if (LUA_OK!=luaL_loadbufferx(L, luaapitest_src.c_str(), luaapitest_src.size(), "test", "t")) {
      hello.name = "Cannot load API test script";
      return;
    }
    if (LUA_OK!=lua_pcall(L, 0, 0, 0)) {
      hello.name = "Cannot execute API test script:\n";
      hello.name += luaL_optlstring(L, -1, "uknown", 0);
      return;
    }
    hello.name = "everything OK!";
    lua_close(L);
  };
  if (ImGui::Begin("My Demo", opened)) {
    if (ImGuiInspect(hello, modified)) {
      ImGui::Text("Modified Items:");
      for (auto const& item: modified)
        ImGui::Text("%s", item.c_str());
    }
  }
  ImGui::End();

  if (ImGui::Begin("Dynamic Parms Test", opened)) {
    if (dynaParms().loaded()) {
      dynaParms().updateInspector();
      for (auto entry: dynaParms().dirtyEntries()) {
        printf("modified: %s\n", entry.c_str());
        if (auto parm=dynaParms().get(entry)) {
          if (parm->is<std::string>())
            printf("%s = %s\n", entry.c_str(), parm->as<std::string>().c_str());
          else if (parm->is<int>())
            printf("%s = %d\n", entry.c_str(), parm->as<int>());
          else if (parm->is<float>())
            printf("%s = %f\n", entry.c_str(), parm->as<float>());
          else if (parm->is<Parm::float2>()) {
            auto p = parm->as<Parm::float2>();
            printf("%s = %f, %f\n", entry.c_str(), p.x, p.y);
          } else if (parm->is<Parm::float3>()) {
            auto p = parm->as<Parm::float3>();
            printf("%s = %f, %f, %f\n", entry.c_str(), p.x, p.y, p.z);
          }
        }
      }
    }
  } 
  ImGui::End();
}

}