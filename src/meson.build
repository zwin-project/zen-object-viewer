_sources = [
  'file-reader.cc',
  'foundation.cc',
  'viewer.cc',
  'main.cc',

  shaders['default']['vert'],
  shaders['default']['frag'],
  shaders['floor']['vert'],
  shaders['floor']['frag'],
]

_viewer_dep = [
  glm_dep,
  zukou_dep,
]

executable(
  'zen-object-viewer',
  _sources,
  cpp_args: ['-include', 'config.h'],
  install: true,
  dependencies: [ _viewer_dep ],
)
