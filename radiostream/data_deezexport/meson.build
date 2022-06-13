project('generic', 'cpp',default_options: ['buildtype=debug','c_std=c11', 'cpp_std=c++17'])
inc = include_directories('src')
curllib = dependency('libcurl',
  required: true,
  static: false,
  method: 'pkg-config')
glob = run_command('python3', 'src_batch.py')
src = glob.stdout().strip().split('\n')
dir_base = meson.current_source_dir()
dir_install = join_paths(dir_base, 'compile_dir')
executable('tool',install: true,install_dir: dir_install,dependencies : curllib,
 include_directories : [inc],sources : [src])


