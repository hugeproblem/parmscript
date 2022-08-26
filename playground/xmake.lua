includes('..')

set_languages('cxx17', 'c11')
if is_mode("debug") then
  set_symbols("debug")
end

target('playground')
  set_kind('binary')
  add_rules('parmscript')
  add_includedirs('..', '../deps/imgui', '../deps/imgui/misc/cpp', '../deps/imgui/backends', '../deps/lua')
  add_files('../deps/imgui/backends/imgui_impl_dx11.cpp', '../deps/imgui/backends/imgui_impl_win32.cpp')
  add_files('entry.cpp', '../deps/imgui/examples/example_win32_directx11/main.cpp')
  add_files('parms/*.lua')
  add_deps('imgui', 'parmscript')
  add_links('d3d11', 'd3dcompiler', 'gdi32', 'user32', 'shell32', 'advapi32')
