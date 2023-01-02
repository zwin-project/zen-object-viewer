gltf_vert_glsl = custom_target(
  'gltf.vertex.h',
  command: [textify, 'gltf_vertex_shader_source', '@INPUT@', '@OUTPUT@'],
  input: 'gltf.vert',
  output: 'gltf.vertex.h',
)

gltf_frag_glsl = custom_target(
  'gltf.fragment.h',
  command: [textify, 'gltf_fragment_shader_source', '@INPUT@', '@OUTPUT@'],
  input: 'gltf.frag',
  output: 'gltf.fragment.h',
)

_sources = [
  '../../include/tiny_gltf.cc',
  'main.cc',
  gltf_vert_glsl,
  gltf_frag_glsl,
]

_viewer_dep = [
  glm_dep,
  zukou_dep,
]

executable(
  'zen-gltf-viewer',
  _sources,
  install: true,
  dependencies: [ _viewer_dep ],
  include_directories: public_inc,
)
