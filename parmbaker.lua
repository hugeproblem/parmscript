local fmt=string.format
local printf=function(...) print(fmt(...)) end

local pe=require('parmexpr')

local input=io.stdin
local headerpath, implpath
local outdir='./'
local skipnext=false
if #arg>0 then
  for i=1,#arg do
    if skipnext then
      skipnext=false
    elseif i<#arg and (arg[i]=='-H' or arg[i]=='--header') then
      headerpath=arg[i+1]
      skipnext=true
    elseif i<#arg and (arg[i]=='-I' or arg[i]=='--impl') then
      implpath=arg[i+1]
      skipnext=true
    else
      input=io.open(arg[i])
      local namepart=arg[i]:find('[\\/][^\\/]+$')
      if namepart then
        outdir=arg[i]:sub(1,namepart)
        printf('default output dir is "%s"', outdir)
      end
    end
  end
end

local ps=pe(input:read('*all'))

if ps then
  local name=ps.parmsetName()

  headerpath=headerpath or outdir..name:lower()..'.h'
  implpath=implpath or outdir..name:lower()..'_imgui.inl'

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


