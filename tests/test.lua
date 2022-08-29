setmetatable(_G,{
  __newindex=function(name,value)
    print(string.format('accessing global variable %s', name))
  end})
package.path=package.path..';../parmscript/?.lua'
local pe=require('parmexpr')

local ps=pe([[
parmset 'Hello'

label 'A Little Test:'
toggle 'x' {label='Enable Group Foo', default=true}
text 'name' {label='Name', default='Foo\nBar\n!@#$%^&*()_+""', multiline=true}
group 'foo' {label='Foo', disablewhen='{x}==false'}
  label 'A Int Value:' {joinnext=true}
  int 'a' {max=1024, min=1}
  float 'b' {default=1024, ui='drag', speed=1}
  color 'color1' {hdr=true, default={1,0,0}}
  color 'color2' {alpha=false, hsv=true, wheel=true, default={0.3,0.1,0.4,0.5}}
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
  float2 'pos' {disablewhen='{Points}.empty()'}
  menu 'mode' {
    class='Mode', label='Mode',
    items={'a','b','c'}, default='b',
    itemlabels={'Apple','Banana','Coffe'},
    itemvalues={4,8,16} }
  color 'color3' {default={0.8,0.2,0.2,1.0}, disablewhen='{mode}!={menu:mode::a}'}
endgroup 'bar'
list 'Points' {class='Point'}
  float3 "pos" {label="Position"}
  float3 "N" {label="Normal", default={0,1,0}}
endlist 'Points'
]])

print('//---------FIELDS----------')
for i,v in pairs(ps.allParms()) do
  print(i,v)
end
ps.setUseBuiltinTypes() -- use float[4] to represent float4, and so forth
print('//----------CPP STRUCT-----------')
print(ps.cppStruct())
print('//----------IMGUI INSPECTOR----------')
print(ps.imguiInspector('parms'))
