project(
  'zen object viewer', 'c', 'cpp',
  version: '0.0.1',
  license: 'MIT',
  meson_version: '>= 0.58.0',
  default_options: ['warning_level=3', 'werror=true', 'optimization=2', 'c_std=gnu11', 'cpp_std=gnu++17'],
)

cxx = meson.get_compiler('cpp')
cdata = configuration_data()

global_args = []
global_args_maybe = [
  '-D_GNU_SOURCE',
  '-DWL_HIDE_DEPRECATED',
  '-fvisibility=hidden',
  '-Wstrict-prototypes',
  '-Wno-gnu-zero-variadic-macro-arguments',
]

foreach arg : global_args_maybe
  if cxx.has_argument(arg)
    global_args += arg
  endif
endforeach
add_project_arguments(global_args, language: 'cpp')

wayland_req = '>= 1.18.0'

glm_dep = dependency('glm')
zukou_dep = dependency('zukou')
jpeg_dep = dependency('libjpeg')

pkgconfig = import('pkgconfig')

textify = files('tools/textify.sh')[0]

public_inc = include_directories('include')

configure_file(
  output: 'config.h',
  configuration: cdata,
)

subdir('src')
