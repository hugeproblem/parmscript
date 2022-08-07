rule('parmscript')
  set_extensions('.parm', '.lua')
  on_load(function(target)
    local outdir = path.join(target:autogendir(), "parms")
    if not os.isdir(outdir) then
      os.mkdir(outdir)
    end
    target:add("includedirs", outdir)
  end)
  before_buildcmd_file(function(target, batchcmds, srcfile, opt)
    local outdir = path.join(target:autogendir(), "parms")
    target:add("includedirs", outdir)

    batchcmds:show_progress(opt.progress, "${color.build.object}processing \"%s\"", srcfile)
    local name=srcfile:match('[\\/]?(%w+)%.%w+$')
    local headerpath=path.join(outdir, name:lower()..'.h')
    local implpath=path.join(outdir, name:lower()..'_imgui_inspector.inl')
    --xmake sandboxed `load` function, which parmexpr depends on, so we have to call lua ... instead
    --[[
    local pe = import('parmexpr', {rootdir='..'})
    local parmsript = io.open(srcfile)
    local ps

    if parmsript then
      ps = pe(parmsript:read('*all'))
    end

    if not ps then
      -- error out
    end

    local name=ps.parmsetName()
    local headerpath=path.join(outdir, name:lower()..'.h')
    local implpath=path.join(outdir, name:lower()..'_imgui.inl')

    batchcmds:mkdir(outdir)

    batchcmds:show_progress(opt.progress, "${color.build.object}generating %s", headerpath)
    ps.setUseBuiltinTypes()
    local header=io.open(headerpath, 'w')
    header:write('//Generated by Parmscript\n#pragma once\n#include <string>\n#include <unordered_set>\n#include <vector>\n\n')
    header:write(ps.cppStruct())
    header:write('\n')
    header:write(fmt('bool ImGuiInspect(%s&, std::unordered_set<std::string>&);\n', name))
    header:write('\n')
    header:close()

    batchcmds:show_progress(opt.progress, "${color.build.object}generating %s", implpath)
    local impl=io.open(implpath, 'w')
    impl:write('//Generated by Parmscript\n')
    impl:write(ps.imguiInspector())
    impl:close()
    ]]

    local args = {'../parmbaker.lua', '-H', path(headerpath), '-I', path(implpath), path(srcfile)}
    batchcmds:vrunv('../lua54', args)
    batchcmds:add_depfiles(srcfile, headerpath, implpath)
  end)

target('imgui')
  set_kind('static')
  add_includedirs('imgui')
  add_files('imgui/*.cpp|imgui_demo.cpp', 'imgui/misc/cpp/*.cpp')
  add_files('imgui/backends/imgui_impl_dx11.cpp', 'imgui/backends/imgui_impl_win32.cpp')

target('playground')
  set_kind('binary')
  add_rules('parmscript')
  add_includedirs('imgui', 'imgui/misc/cpp', 'imgui/backends')
  add_files('entry.cpp', 'imgui/examples/example_win32_directx11/main.cpp')
  add_files('parms/*.lua')
  add_deps('imgui')
  add_links('d3d11', 'd3dcompiler', 'gdi32', 'user32', 'shell32', 'advapi32')
