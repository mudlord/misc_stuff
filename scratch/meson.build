project('scratch', 'cpp',default_options: [ 'buildtype=release', 'c_std=c11','cpp_std=c++17'])
if host_machine.system() == 'linux'
sdl2dep = dependency('sdl2',
  required: true,
  static: false,
  method: 'pkg-config')
else
sdl2dep = dependency('sdl2',
  required: true,
  static: true,
  method: 'pkg-config')
endif
inc = include_directories('src')
inc_imgui = include_directories('src/deps/imgui')
inc_deps = include_directories('src/deps')
inc_stb = include_directories('src/deps/stb')

glob = run_command('python3', 'src_batch.py')
src = glob.stdout().strip().split('\n')

if host_machine.system() == 'linux'
    dl_dep = meson.get_compiler('cpp').find_library('dl', required: true)
else
    dl_dep = dependency('', required: false)
endif

test_ldflags = []
common_ldflags = meson.get_compiler('cpp').get_supported_link_arguments (
  test_ldflags)

if host_machine.system() == 'windows'
  common_ldflags += [
    '-fno-stack-protector',
    '-lws2_32',
    '-static'
    ]
endif
add_project_link_arguments (
common_ldflags,
language: [ 'c', 'cpp' ],)
dir_base = meson.current_source_dir()
dir_install = join_paths(dir_base, 'compile_dir')
executable('scratch',install: true,install_dir: dir_install,include_directories : [inc,inc_imgui,inc_deps,inc_stb],sources : [src],dependencies : [sdl2dep,dl_dep]
)

