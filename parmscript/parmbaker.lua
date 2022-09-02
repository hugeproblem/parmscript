local fmt=string.format
local printf=function(...) print(fmt(...)) end

local pe=require('parmexpr')

local helpstr = [[
================
   PARM BAKER
================

usage:
  lua parmbaker.lua [-H path] [-I path] [-C] [-N name] [parmscript]

args:

-H, --header path    : output header file path
-I, --impl   path    : output implement file path
-C, --cpp            : output implement as .cpp (otherwise outputs .inl)
-N, --namespace name : output in namespace `name`
parmscript           : the parmscript input file, reads from stdin if missing

]]

local input=io.stdin
local headerpath, implpath
local outputcpp=false
local outdir='./'
local skipnext=false
local beginnamespace,endnamespace='',''
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
    elseif arg[i]=='-C' or arg[i]=='--cpp' then
      outputcpp=true
    elseif arg[i]=='-N' or arg[i]=='--namespace' then
      local namespace=arg[i+1]
      skipnext=true
      beginnamespace=fmt('namespace %s {\n', namespace)
      endnamespace='}\n'
    elseif arg[i]=='-h' or arg[i]=='--help' then
      print(helpstr)
      os.exit(0)
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
  if not implpath then
    if outputcpp then
      implpath=outdir..name:lower()..'_imgui.cpp'
    else
      implpath=outdir..name:lower()..'_imgui.inl'
    end
  end

  printf('writing header to "%s"', headerpath)
  local header=io.open(headerpath, 'w')
  ps.setUseBuiltinTypes()
  header:write('//Generated by Parmscript\n#pragma once\n')
  header:write('#include <functional>\n#include <string>\n#include <unordered_set>\n#include <vector>\n\n')
  header:write(beginnamespace)
  header:write(ps.cppStruct())
  header:write('\n')
  if outputcpp then
    header:write(fmt('bool ImGuiInspect(%s&, std::unordered_set<std::string>&);\n', name))
    header:write('\n')
  end
  header:write(endnamespace)
  header:close()

  printf('writing impl to "%s"', implpath)
  local impl=io.open(implpath, 'w')
  impl:write('//Generated by Parmscript\n')
  if outputcpp then
    impl:write(fmt('#include "%s"\n', name:lower()..'.h'))
    impl:write('#include <imgui.h>\n#include <imgui_stdlib.h>\n')
  end
  impl:write(beginnamespace)
  impl:write(ps.imguiInspector())
  impl:write(endnamespace)
  impl:close()
else
  printf('error: cannot parse input')
end

