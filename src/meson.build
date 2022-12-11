default_vert_glsl = custom_target(
  'default.vertex.h',
  command: [textify, 'default_vertex_shader_source', '@INPUT@', '@OUTPUT@'],
  input: 'default.vert',
  output: 'default.vert.h',
)

default_frag_glsl = custom_target(
  'default.fragment.h',
  command: [textify, 'default_fragment_shader_source', '@INPUT@', '@OUTPUT@'],
  input: 'default.frag',
  output: 'default.fragment.h',
)

_sources = [
  'file-reader.cc',
  'main.cc',
  default_vert_glsl,
  default_frag_glsl,
]

_viewer_dep = [
  glm_dep,
  zukou_dep,
]

executable(
  'zen-object-viewer',
  _sources,
  install: true,
  dependencies: [ _viewer_dep ],
)
