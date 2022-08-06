local fmt=string.format
local printf=function(...) print(fmt(...)) end

local pe=require('parmexpr')

local input=io.stdin
local outdir='./'
if #arg>0 then
  input=io.open(arg[1])
  local namepart=arg[1]:find('[\\/][^\\/]+$')
  if namepart then
    outdir=arg[1]:sub(1,namepart)
    printf('output to "%s"', outdir)
  end
end

local ps=pe(input:read('*all'))

if ps then
  local name=ps.parmsetName()
  local headerpath=outdir..name:lower()..'.h'
  local implpath=outdir..name:lower()..'_imgui.inl'

  printf('writing header to "%s"', headerpath)
  local header=io.open(headerpath, 'w')
  ps.setUseBuiltinTypes()
  header:write('//Generated by Parmscript\n#pragma once\n#include <string>\n#include <unordered_set>\n#include <vector>\n\n')
  header:write(ps.cppStruct())
  header:write('\n')
  header:write(fmt('bool ImGuiInspect(%s&, std::unordered_set<std::string>&);\n', name))
  header:write('\n')
  header:close()

  printf('writing impl to "%s"', implpath)
  local impl=io.open(implpath, 'w')
  impl:write('//Generated by Parmscript\n')
  impl:write(ps.imguiInspector())
  impl:close()
else
  printf('error: cannot parse input')
end

