set_languages('cxx17', 'c11')
if is_mode("debug") then
  set_symbols("debug")
end

target('imgui')
  set_kind('static')
  add_includedirs('deps/imgui')
  add_files('deps/imgui/*.cpp|imgui_demo.cpp', 'deps/imgui/misc/cpp/*.cpp')

target('lua')
  set_kind('static')
  add_includedirs('deps/lua')
  add_files('deps/lua/*.c|lua.c|luac.c|onelua.c')

target('luaexe')
  set_basename('lua54')
  set_kind('binary')
  add_includedirs('deps/lua')
  add_files('deps/lua/lua.c')
  add_deps('lua')

target('parmscript')
  set_kind('static')
  add_includedirs('.', 'deps/imgui', 'deps/imgui/misc/cpp', 'deps/imgui/backends', 'deps/lua', 'deps/sol2/include')
  if is_mode("debug") then
    add_defines("DEBUG")
  end
  add_files('parmscript.cpp')
  add_files('parmexpr.lua', {rule='utils.bin2c'})
  add_deps('imgui', 'lua')

rule('parmscript')
  set_extensions('.parm', '.lua')
  on_load(function(target)
    local outdir = path.join(target:autogendir(), "parms")
    if not os.isdir(outdir) then
      os.mkdir(outdir)
    end
    target:add('deps', 'luaexe')
    target:add("includedirs", target:autogendir())
  end)
  before_buildcmd_file(function(target, batchcmds, srcfile, opt)
    import('core.project.project')
    local outdir = path.join(target:autogendir(), "parms")
    target:add("includedirs", target:autogendir())

    batchcmds:show_progress(opt.progress, "${color.build.object}processing \"%s\"", srcfile)
    local name=srcfile:match('[\\/]?(%w+)%.%w+$')
    local headerpath=path.join(outdir, name:lower()..'.h')
    local implpath=path.join(outdir, name:lower()..'_imgui_inspector.cpp')

    local args = {'-e', 'package.path="'..os.scriptdir():gsub('\\','/')..'/?.lua"',
                  path.join(os.scriptdir(), 'parmbaker.lua'),
                  '-H', path(headerpath), '-I', path(implpath), '--cpp',
                  os.projectdir()..'/'..path(srcfile)}
    batchcmds:vrunv(path.join(os.scriptdir(), project.target('luaexe'):targetfile()), args)

    local objfile=target:objectfile(implpath)
    table.insert(target:objectfiles(), objfile)
    batchcmds:compile(implpath, objfile)

    batchcmds:add_depfiles(srcfile,headerpath,implpath)
    batchcmds:set_depmtime(os.mtime(objfile))
    batchcmds:set_depcache(target:dependfile(objfile))
  end)

